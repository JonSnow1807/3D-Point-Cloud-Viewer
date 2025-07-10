#include "processing/VoxelDownsampling.h"
#include <cmath>

namespace pcv {

void VoxelDownsampling::Voxel::addPoint(const Point& point) {
    position_sum += point.position;
    color_sum += point.color;
    normal_sum += point.normal;
    intensity_sum += point.intensity;
    point_count++;
}

Point VoxelDownsampling::Voxel::getRepresentative() const {
    Point rep;
    if (point_count > 0) {
        rep.position = position_sum / static_cast<float>(point_count);
        rep.color = color_sum / static_cast<float>(point_count);
        rep.normal = glm::normalize(normal_sum);
        rep.intensity = intensity_sum / static_cast<float>(point_count);
    }
    return rep;
}

void VoxelDownsampling::downsample(PointCloud& cloud, const Parameters& params) {
    auto downsampled = createDownsampled(cloud, params);
    cloud = *downsampled;
}

PointCloud::Ptr VoxelDownsampling::createDownsampled(const PointCloud& cloud, 
                                                     const Parameters& params) {
    auto result = std::make_shared<PointCloud>();
    
    if (cloud.empty() || params.leaf_size <= 0.0f) {
        *result = cloud;
        return result;
    }
    
    // Build voxel grid
    VoxelGrid grid = buildVoxelGrid(cloud, params.leaf_size);
    
    // Reserve space for result
    result->reserve(grid.size());
    
    // Extract representative points
    for (const auto& [key, voxel] : grid) {
        result->addPoint(voxel.getRepresentative());
    }
    
    return result;
}

VoxelDownsampling::Statistics VoxelDownsampling::getStatistics(const PointCloud& cloud, 
                                                               const Parameters& params) {
    Statistics stats;
    stats.original_points = cloud.size();
    
    if (cloud.empty() || params.leaf_size <= 0.0f) {
        stats.downsampled_points = stats.original_points;
        stats.voxel_count = stats.original_points;
        stats.compression_ratio = 1.0f;
        return stats;
    }
    
    // Build voxel grid to count voxels
    VoxelGrid grid = buildVoxelGrid(cloud, params.leaf_size);
    
    stats.voxel_count = grid.size();
    stats.downsampled_points = stats.voxel_count;
    stats.compression_ratio = stats.original_points > 0 ? 
        static_cast<float>(stats.downsampled_points) / stats.original_points : 1.0f;
    
    return stats;
}

VoxelDownsampling::VoxelKey VoxelDownsampling::computeVoxelKey(const glm::vec3& point, 
                                                               float leaf_size) {
    VoxelKey key;
    key.x = static_cast<int>(std::floor(point.x / leaf_size));
    key.y = static_cast<int>(std::floor(point.y / leaf_size));
    key.z = static_cast<int>(std::floor(point.z / leaf_size));
    return key;
}

VoxelDownsampling::VoxelGrid VoxelDownsampling::buildVoxelGrid(const PointCloud& cloud, 
                                                               float leaf_size) {
    VoxelGrid grid;
    
    for (const auto& point : cloud) {
        VoxelKey key = computeVoxelKey(point.position, leaf_size);
        grid[key].addPoint(point);
    }
    
    return grid;
}

} // namespace pcv