#include "processing/Filters.h"

namespace pcv {

FilterPipeline& FilterPipeline::addVoxelDownsampling(float leaf_size) {
    Filter filter;
    filter.type = Filter::VOXEL;
    filter.voxel_params.leaf_size = leaf_size;
    filters_.push_back(filter);
    return *this;
}

FilterPipeline& FilterPipeline::addStatisticalOutlierRemoval(int k_neighbors, float std_multiplier) {
    Filter filter;
    filter.type = Filter::STATISTICAL_OUTLIER;
    filter.statistical_params.k_neighbors = k_neighbors;
    filter.statistical_params.std_multiplier = std_multiplier;
    filters_.push_back(filter);
    return *this;
}

FilterPipeline& FilterPipeline::addRadiusOutlierRemoval(float radius, int min_neighbors) {
    Filter filter;
    filter.type = Filter::RADIUS_OUTLIER;
    filter.radius_params.radius = radius;
    filter.radius_params.min_neighbors = min_neighbors;
    filters_.push_back(filter);
    return *this;
}

void FilterPipeline::apply(PointCloud& cloud) const {
    for (const auto& filter : filters_) {
        switch (filter.type) {
            case Filter::VOXEL:
                VoxelDownsampling::downsample(cloud, filter.voxel_params);
                break;
            case Filter::STATISTICAL_OUTLIER:
                OutlierRemoval::removeStatisticalOutliers(cloud, filter.statistical_params);
                break;
            case Filter::RADIUS_OUTLIER:
                OutlierRemoval::removeRadiusOutliers(cloud, filter.radius_params);
                break;
        }
    }
}

PointCloud::Ptr FilterPipeline::apply(const PointCloud& cloud) const {
    auto result = std::make_shared<PointCloud>(cloud);
    apply(*result);
    return result;
}

} // namespace pcv