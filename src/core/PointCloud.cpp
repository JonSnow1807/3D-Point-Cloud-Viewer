#include "core/PointCloud.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>

namespace pcv {

PointCloud::PointCloud(size_t reserve_size) {
    points_.reserve(reserve_size);
}

void PointCloud::addPoint(const Point& point) {
    points_.push_back(point);
    updateBounds(point);
}

void PointCloud::addPoint(const glm::vec3& position) {
    Point p(position);
    addPoint(p);
}

void PointCloud::addPoint(const glm::vec3& position, const glm::vec3& color) {
    Point p(position, color);
    addPoint(p);
}

Point& PointCloud::at(size_t idx) {
    if (idx >= points_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return points_[idx];
}

const Point& PointCloud::at(size_t idx) const {
    if (idx >= points_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return points_[idx];
}

float PointCloud::getDiagonalLength() const {
    return glm::length(max_bound_ - min_bound_);
}

void PointCloud::transform(const glm::mat4& transformation) {
    for (auto& point : points_) {
        glm::vec4 pos(point.position, 1.0f);
        point.position = glm::vec3(transformation * pos);
        
        // Transform normal (use inverse transpose for correct normal transformation)
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transformation)));
        point.normal = glm::normalize(normalMatrix * point.normal);
    }
    updateBounds();
}

void PointCloud::translateCentroid(const glm::vec3& target) {
    glm::vec3 centroid = getCenter();
    glm::vec3 translation = target - centroid;
    
    for (auto& point : points_) {
        point.position += translation;
    }
    
    min_bound_ += translation;
    max_bound_ += translation;
}

void PointCloud::scale(float factor) {
    glm::vec3 center = getCenter();
    
    for (auto& point : points_) {
        point.position = center + factor * (point.position - center);
    }
    
    updateBounds();
}

void PointCloud::computeNormals(int k_neighbors) {
    // Simplified normal computation - in production, use KD-tree for neighbor search
    // For now, we'll use a placeholder implementation
    for (auto& point : points_) {
        // This is a placeholder - real implementation would use PCA on k-nearest neighbors
        point.normal = glm::normalize(point.position);
    }
}

bool PointCloud::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    clear();
    std::string line;
    
    // Simple XYZ RGB format reader
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        float x, y, z, r = 1.0f, g = 1.0f, b = 1.0f;
        
        if (iss >> x >> y >> z) {
            // Try to read color
            iss >> r >> g >> b;
            addPoint(glm::vec3(x, y, z), glm::vec3(r, g, b));
        }
    }
    
    file.close();
    return !empty();
}

bool PointCloud::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Point Cloud Data\n";
    file << "# Format: X Y Z R G B\n";
    file << "# Points: " << size() << "\n";
    
    for (const auto& point : points_) {
        file << point.position.x << " " 
             << point.position.y << " " 
             << point.position.z << " "
             << point.color.r << " "
             << point.color.g << " "
             << point.color.b << "\n";
    }
    
    file.close();
    return true;
}

size_t PointCloud::getMemoryUsage() const {
    return sizeof(PointCloud) + points_.capacity() * sizeof(Point);
}

void PointCloud::updateBounds() {
    if (points_.empty()) {
        min_bound_ = glm::vec3(0.0f);
        max_bound_ = glm::vec3(0.0f);
        return;
    }
    
    min_bound_ = glm::vec3(std::numeric_limits<float>::max());
    max_bound_ = glm::vec3(std::numeric_limits<float>::lowest());
    
    for (const auto& point : points_) {
        updateBounds(point);
    }
}

void PointCloud::updateBounds(const Point& point) {
    min_bound_ = glm::min(min_bound_, point.position);
    max_bound_ = glm::max(max_bound_, point.position);
}

} // namespace pcv