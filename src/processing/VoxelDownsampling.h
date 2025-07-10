#pragma once

#include "core/PointCloud.h"
#include <unordered_map>

namespace pcv {

class VoxelDownsampling {
public:
    // Voxel grid parameters
    struct Parameters {
        float leaf_size;      // Size of voxel in each dimension
        bool compute_mean;    // Use mean position vs. centroid
        
        Parameters() : leaf_size(0.01f), compute_mean(true) {}
    };
    
    // Apply voxel grid downsampling
    static void downsample(PointCloud& cloud, const Parameters& params = Parameters());
    
    // Create downsampled copy
    static PointCloud::Ptr createDownsampled(const PointCloud& cloud, 
                                             const Parameters& params = Parameters());
    
    // Get voxel grid statistics
    struct Statistics {
        size_t original_points;
        size_t downsampled_points;
        size_t voxel_count;
        float compression_ratio;
    };
    
    static Statistics getStatistics(const PointCloud& cloud, 
                                   const Parameters& params = Parameters());
    
private:
    // Voxel key for spatial hashing
    struct VoxelKey {
        int x, y, z;
        
        bool operator==(const VoxelKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };
    
    // Hash function for VoxelKey
    struct VoxelKeyHash {
        std::size_t operator()(const VoxelKey& key) const {
            // Simple hash combination
            return ((std::hash<int>()(key.x) ^ 
                    (std::hash<int>()(key.y) << 1)) >> 1) ^ 
                    (std::hash<int>()(key.z) << 1);
        }
    };
    
    // Voxel data
    struct Voxel {
        glm::vec3 position_sum{0.0f};
        glm::vec3 color_sum{0.0f};
        glm::vec3 normal_sum{0.0f};
        float intensity_sum = 0.0f;
        size_t point_count = 0;
        
        void addPoint(const Point& point);
        Point getRepresentative() const;
    };
    
    using VoxelGrid = std::unordered_map<VoxelKey, Voxel, VoxelKeyHash>;
    
    static VoxelKey computeVoxelKey(const glm::vec3& point, float leaf_size);
    static VoxelGrid buildVoxelGrid(const PointCloud& cloud, float leaf_size);
};

} // namespace pcv