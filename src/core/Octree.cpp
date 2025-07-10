#include "core/Octree.h"
#include <algorithm>
#include <glm/geometric.hpp>

namespace pcv {

// OctreeNode Implementation
OctreeNode::OctreeNode(const glm::vec3& min_bound, const glm::vec3& max_bound, int depth)
    : min_bound_(min_bound), max_bound_(max_bound), depth_(depth) {
}

void OctreeNode::insertPoint(size_t point_index, const glm::vec3& position, const PointCloud& cloud) {
    if (isLeaf()) {
        point_indices_.push_back(point_index);
        
        // Check if we need to subdivide
        if (point_indices_.size() > MAX_POINTS_PER_LEAF && depth_ < MAX_DEPTH) {
            subdivide(cloud);
        }
    } else {
        // Insert into appropriate child
        int octant = getOctant(position);
        children_[octant]->insertPoint(point_index, position, cloud);
    }
}

void OctreeNode::subdivide(const PointCloud& cloud) {
    glm::vec3 center = getCenter();
    
    // Create 8 children
    for (int i = 0; i < 8; ++i) {
        glm::vec3 child_min = min_bound_;
        glm::vec3 child_max = max_bound_;
        
        if (i & 1) child_min.x = center.x; else child_max.x = center.x;
        if (i & 2) child_min.y = center.y; else child_max.y = center.y;
        if (i & 4) child_min.z = center.z; else child_max.z = center.z;
        
        children_[i] = std::make_unique<OctreeNode>(child_min, child_max, depth_ + 1);
    }
    
    // Redistribute points to children
    std::vector<size_t> temp_indices = std::move(point_indices_);
    point_indices_.clear();
    
    for (size_t idx : temp_indices) {
        const glm::vec3& pos = cloud[idx].position;
        int octant = getOctant(pos);
        children_[octant]->insertPoint(idx, pos, cloud);
    }
}

int OctreeNode::getOctant(const glm::vec3& point) const {
    glm::vec3 center = getCenter();
    int octant = 0;
    
    if (point.x > center.x) octant |= 1;
    if (point.y > center.y) octant |= 2;
    if (point.z > center.z) octant |= 4;
    
    return octant;
}

// Octree Implementation
Octree::Octree(const PointCloud& cloud) : cloud_(cloud) {
}

void Octree::build() {
    if (cloud_.empty()) return;
    
    // Create root node with cloud bounds
    root_ = std::make_unique<OctreeNode>(cloud_.getMinBound(), cloud_.getMaxBound());
    
    // Insert all points
    for (size_t i = 0; i < cloud_.size(); ++i) {
        root_->insertPoint(i, cloud_[i].position, cloud_);
    }
}

std::vector<size_t> Octree::queryFrustum(const FrustumPlanes& frustum) const {
    std::vector<size_t> results;
    if (root_) {
        queryFrustumRecursive(root_.get(), frustum, results);
    }
    return results;
}

std::vector<size_t> Octree::queryRadius(const glm::vec3& center, float radius) const {
    std::vector<size_t> results;
    if (root_) {
        queryRadiusRecursive(root_.get(), center, radius, results);
    }
    return results;
}

std::vector<size_t> Octree::queryBox(const glm::vec3& min_bound, const glm::vec3& max_bound) const {
    std::vector<size_t> results;
    if (root_) {
        queryBoxRecursive(root_.get(), min_bound, max_bound, results);
    }
    return results;
}

std::vector<size_t> Octree::queryLOD(const glm::vec3& view_position, 
                                     const FrustumPlanes& frustum,
                                     float base_distance) const {
    std::vector<size_t> results;
    if (root_) {
        queryLODRecursive(root_.get(), view_position, frustum, base_distance, results);
    }
    return results;
}

bool Octree::isNodeInFrustum(const OctreeNode* node, const FrustumPlanes& frustum) const {
    // Check if bounding box intersects frustum
    const glm::vec3& min_bound = node->getMinBound();
    const glm::vec3& max_bound = node->getMaxBound();
    
    for (const auto& plane : frustum) {
        glm::vec3 p_vertex(
            plane.x > 0 ? max_bound.x : min_bound.x,
            plane.y > 0 ? max_bound.y : min_bound.y,
            plane.z > 0 ? max_bound.z : min_bound.z
        );
        
        if (distanceToPlane(p_vertex, plane) < 0) {
            return false; // Outside this plane
        }
    }
    
    return true; // Inside all planes
}

bool Octree::isPointInFrustum(const glm::vec3& point, const FrustumPlanes& frustum) const {
    for (const auto& plane : frustum) {
        if (distanceToPlane(point, plane) < 0) {
            return false;
        }
    }
    return true;
}

float Octree::distanceToPlane(const glm::vec3& point, const glm::vec4& plane) const {
    return plane.x * point.x + plane.y * point.y + plane.z * point.z + plane.w;
}

void Octree::queryFrustumRecursive(const OctreeNode* node, 
                                   const FrustumPlanes& frustum,
                                   std::vector<size_t>& results) const {
    if (!isNodeInFrustum(node, frustum)) {
        return; // Early rejection
    }
    
    if (node->isLeaf()) {
        // Add all points in this leaf that are inside frustum
        for (size_t idx : node->getPointIndices()) {
            if (isPointInFrustum(cloud_[idx].position, frustum)) {
                results.push_back(idx);
            }
        }
    } else {
        // Recurse into children
        for (int i = 0; i < 8; ++i) {
            if (node->getChild(i)) {
                queryFrustumRecursive(node->getChild(i), frustum, results);
            }
        }
    }
}

void Octree::queryRadiusRecursive(const OctreeNode* node,
                                  const glm::vec3& center,
                                  float radius,
                                  std::vector<size_t>& results) const {
    // Check if node bounding box intersects sphere
    glm::vec3 closest = glm::clamp(center, node->getMinBound(), node->getMaxBound());
    float dist_sq = glm::dot(center - closest, center - closest);
    
    if (dist_sq > radius * radius) {
        return; // No intersection
    }
    
    if (node->isLeaf()) {
        // Check each point
        float radius_sq = radius * radius;
        for (size_t idx : node->getPointIndices()) {
            glm::vec3 diff = cloud_[idx].position - center;
            if (glm::dot(diff, diff) <= radius_sq) {
                results.push_back(idx);
            }
        }
    } else {
        // Recurse into children
        for (int i = 0; i < 8; ++i) {
            if (node->getChild(i)) {
                queryRadiusRecursive(node->getChild(i), center, radius, results);
            }
        }
    }
}

void Octree::queryLODRecursive(const OctreeNode* node,
                               const glm::vec3& view_position,
                               const FrustumPlanes& frustum,
                               float base_distance,
                               std::vector<size_t>& results) const {
    if (!isNodeInFrustum(node, frustum)) {
        return;
    }
    
    // Calculate distance from view to node center
    float dist = glm::length(view_position - node->getCenter());
    
    // Determine LOD level based on distance and node size
    float node_size = glm::length(node->getMaxBound() - node->getMinBound());
    float detail_ratio = node_size / dist;
    
    // If node is small enough relative to distance, use simplified representation
    if (detail_ratio < 0.01f || node->getDepth() >= 5) {
        // Add only a subset of points for distant nodes
        const auto& indices = node->getPointIndices();
        if (!indices.empty()) {
            // Simple decimation: take every Nth point based on distance
            int stride = std::max(1, static_cast<int>(dist / base_distance));
            for (size_t i = 0; i < indices.size(); i += stride) {
                results.push_back(indices[i]);
            }
        }
    } else if (node->isLeaf()) {
        // Close enough - add all points
        results.insert(results.end(), 
                      node->getPointIndices().begin(), 
                      node->getPointIndices().end());
    } else {
        // Recurse into children
        for (int i = 0; i < 8; ++i) {
            if (node->getChild(i)) {
                queryLODRecursive(node->getChild(i), view_position, frustum, base_distance, results);
            }
        }
    }
}

void Octree::queryBoxRecursive(const OctreeNode* node,
                               const glm::vec3& min_bound,
                               const glm::vec3& max_bound,
                               std::vector<size_t>& results) const {
    // Check if node bounding box intersects query box
    if (node->getMaxBound().x < min_bound.x || node->getMinBound().x > max_bound.x ||
        node->getMaxBound().y < min_bound.y || node->getMinBound().y > max_bound.y ||
        node->getMaxBound().z < min_bound.z || node->getMinBound().z > max_bound.z) {
        return; // No intersection
    }
    
    if (node->isLeaf()) {
        // Check each point in the leaf
        for (size_t idx : node->getPointIndices()) {
            const glm::vec3& pos = cloud_[idx].position;
            if (pos.x >= min_bound.x && pos.x <= max_bound.x &&
                pos.y >= min_bound.y && pos.y <= max_bound.y &&
                pos.z >= min_bound.z && pos.z <= max_bound.z) {
                results.push_back(idx);
            }
        }
    } else {
        // Recurse into children
        for (int i = 0; i < 8; ++i) {
            if (node->getChild(i)) {
                queryBoxRecursive(node->getChild(i), min_bound, max_bound, results);
            }
        }
    }
}

int Octree::getMaxDepth() const {
    int max_depth = 0;
    size_t total_count = 0;
    size_t leaf_count = 0;
    
    if (root_) {
        countNodesRecursive(root_.get(), total_count, leaf_count, max_depth);
    }
    
    return max_depth;
}

size_t Octree::getNodeCount() const {
    size_t total_count = 0;
    size_t leaf_count = 0;
    int max_depth = 0;
    
    if (root_) {
        countNodesRecursive(root_.get(), total_count, leaf_count, max_depth);
    }
    
    return total_count;
}

size_t Octree::getLeafCount() const {
    size_t total_count = 0;
    size_t leaf_count = 0;
    int max_depth = 0;
    
    if (root_) {
        countNodesRecursive(root_.get(), total_count, leaf_count, max_depth);
    }
    
    return leaf_count;
}

void Octree::countNodesRecursive(const OctreeNode* node, 
                                size_t& total_count, 
                                size_t& leaf_count,
                                int& max_depth) const {
    total_count++;
    max_depth = std::max(max_depth, node->getDepth());
    
    if (node->isLeaf()) {
        leaf_count++;
    } else {
        for (int i = 0; i < 8; ++i) {
            if (node->getChild(i)) {
                countNodesRecursive(node->getChild(i), total_count, leaf_count, max_depth);
            }
        }
    }
}

} // namespace pcv