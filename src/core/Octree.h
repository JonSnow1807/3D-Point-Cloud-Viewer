#pragma once

#include "core/PointCloud.h"
#include <array>
#include <memory>
#include <functional>

namespace pcv {

class OctreeNode {
public:
    using Ptr = std::unique_ptr<OctreeNode>;
    
    OctreeNode(const glm::vec3& min_bound, const glm::vec3& max_bound, int depth = 0);
    
    // Node properties
    bool isLeaf() const { return children_[0] == nullptr; }
    int getDepth() const { return depth_; }
    const glm::vec3& getMinBound() const { return min_bound_; }
    const glm::vec3& getMaxBound() const { return max_bound_; }
    glm::vec3 getCenter() const { return (min_bound_ + max_bound_) * 0.5f; }
    
    // Point management
    void insertPoint(size_t point_index, const glm::vec3& position, const PointCloud& cloud);
    const std::vector<size_t>& getPointIndices() const { return point_indices_; }
    size_t getPointCount() const { return point_indices_.size(); }
    
    // Children access
    OctreeNode* getChild(int index) { return children_[index].get(); }
    const OctreeNode* getChild(int index) const { return children_[index].get(); }
    
private:
    glm::vec3 min_bound_;
    glm::vec3 max_bound_;
    int depth_;
    
    static constexpr int MAX_POINTS_PER_LEAF = 100;
    static constexpr int MAX_DEPTH = 10;
    
    std::array<Ptr, 8> children_;
    std::vector<size_t> point_indices_;
    
    void subdivide(const PointCloud& cloud);
    int getOctant(const glm::vec3& point) const;
};

class Octree {
public:
    using FrustumPlanes = std::array<glm::vec4, 6>;
    
    explicit Octree(const PointCloud& cloud);
    
    // Build the octree
    void build();
    
    // Queries
    std::vector<size_t> queryFrustum(const FrustumPlanes& frustum) const;
    std::vector<size_t> queryRadius(const glm::vec3& center, float radius) const;
    std::vector<size_t> queryBox(const glm::vec3& min_bound, const glm::vec3& max_bound) const;
    
    // LOD support
    std::vector<size_t> queryLOD(const glm::vec3& view_position, 
                                 const FrustumPlanes& frustum,
                                 float base_distance = 10.0f) const;
    
    // Statistics
    int getMaxDepth() const;
    size_t getNodeCount() const;
    size_t getLeafCount() const;
    
private:
    const PointCloud& cloud_;
    std::unique_ptr<OctreeNode> root_;
    
    // Helper functions for frustum culling
    bool isNodeInFrustum(const OctreeNode* node, const FrustumPlanes& frustum) const;
    bool isPointInFrustum(const glm::vec3& point, const FrustumPlanes& frustum) const;
    float distanceToPlane(const glm::vec3& point, const glm::vec4& plane) const;
    
    // Recursive query helpers
    void queryFrustumRecursive(const OctreeNode* node, 
                               const FrustumPlanes& frustum,
                               std::vector<size_t>& results) const;
    
    void queryRadiusRecursive(const OctreeNode* node,
                              const glm::vec3& center,
                              float radius,
                              std::vector<size_t>& results) const;
    
    void queryBoxRecursive(const OctreeNode* node,
                           const glm::vec3& min_bound,
                           const glm::vec3& max_bound,
                           std::vector<size_t>& results) const;
    
    void queryLODRecursive(const OctreeNode* node,
                           const glm::vec3& view_position,
                           const FrustumPlanes& frustum,
                           float base_distance,
                           std::vector<size_t>& results) const;
    
    // Statistics helpers
    void countNodesRecursive(const OctreeNode* node, 
                            size_t& total_count, 
                            size_t& leaf_count,
                            int& max_depth) const;
};

} // namespace pcv