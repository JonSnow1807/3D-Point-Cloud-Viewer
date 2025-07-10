#pragma once

#include "core/PointCloud.h"
#include "processing/OutlierRemoval.h"
#include "processing/VoxelDownsampling.h"

namespace pcv {

// Filter pipeline for applying multiple filters in sequence
class FilterPipeline {
public:
    FilterPipeline() = default;
    
    // Add filters to pipeline
    FilterPipeline& addVoxelDownsampling(float leaf_size);
    FilterPipeline& addStatisticalOutlierRemoval(int k_neighbors, float std_multiplier);
    FilterPipeline& addRadiusOutlierRemoval(float radius, int min_neighbors);
    
    // Apply all filters
    void apply(PointCloud& cloud) const;
    PointCloud::Ptr apply(const PointCloud& cloud) const;
    
    // Clear pipeline
    void clear() { filters_.clear(); }
    
private:
    struct Filter {
        enum Type { VOXEL, STATISTICAL_OUTLIER, RADIUS_OUTLIER };
        Type type;
        
        // Use separate members instead of union
        VoxelDownsampling::Parameters voxel_params;
        OutlierRemoval::StatisticalParams statistical_params;
        OutlierRemoval::RadiusParams radius_params;
    };
    
    std::vector<Filter> filters_;
};

} // namespace pcv