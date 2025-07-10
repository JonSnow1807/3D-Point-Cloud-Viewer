#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace pcv {

struct Point {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    float intensity;
    
    Point() : position(0.0f), color(1.0f), normal(0.0f, 0.0f, 1.0f), intensity(1.0f) {}
    Point(const glm::vec3& pos) : position(pos), color(1.0f), normal(0.0f, 0.0f, 1.0f), intensity(1.0f) {}
    Point(const glm::vec3& pos, const glm::vec3& col) 
        : position(pos), color(col), normal(0.0f, 0.0f, 1.0f), intensity(1.0f) {}
};

class PointCloud {
public:
    using Ptr = std::shared_ptr<PointCloud>;
    using ConstPtr = std::shared_ptr<const PointCloud>;
    
    PointCloud() = default;
    explicit PointCloud(size_t reserve_size);
    ~PointCloud() = default;
    
    // Point access
    void addPoint(const Point& point);
    void addPoint(const glm::vec3& position);
    void addPoint(const glm::vec3& position, const glm::vec3& color);
    
    Point& operator[](size_t idx) { return points_[idx]; }
    const Point& operator[](size_t idx) const { return points_[idx]; }
    
    Point& at(size_t idx);
    const Point& at(size_t idx) const;
    
    // Container operations
    size_t size() const { return points_.size(); }
    bool empty() const { return points_.empty(); }
    void clear() { points_.clear(); updateBounds(); }
    void reserve(size_t size) { points_.reserve(size); }
    void resize(size_t size) { points_.resize(size); updateBounds(); }
    
    // Iterators
    std::vector<Point>::iterator begin() { return points_.begin(); }
    std::vector<Point>::iterator end() { return points_.end(); }
    std::vector<Point>::const_iterator begin() const { return points_.begin(); }
    std::vector<Point>::const_iterator end() const { return points_.end(); }
    
    // Data access
    const std::vector<Point>& getPoints() const { return points_; }
    std::vector<Point>& getPoints() { return points_; }
    
    // Bounds
    const glm::vec3& getMinBound() const { return min_bound_; }
    const glm::vec3& getMaxBound() const { return max_bound_; }
    glm::vec3 getCenter() const { return (min_bound_ + max_bound_) * 0.5f; }
    float getDiagonalLength() const;
    
    // Operations
    void transform(const glm::mat4& transformation);
    void translateCentroid(const glm::vec3& target = glm::vec3(0.0f));
    void scale(float factor);
    void computeNormals(int k_neighbors = 10);
    
    // I/O
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    
    // Memory info
    size_t getMemoryUsage() const;
    
private:
    std::vector<Point> points_;
    glm::vec3 min_bound_{std::numeric_limits<float>::max()};
    glm::vec3 max_bound_{std::numeric_limits<float>::lowest()};
    
    void updateBounds();
    void updateBounds(const Point& point);
};

} // namespace pcv