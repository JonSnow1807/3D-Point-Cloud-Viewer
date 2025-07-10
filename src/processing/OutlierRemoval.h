#pragma once

#include "core/PointCloud.h"
#include <vector>

namespace pcv {

class OutlierRemoval {
public:
    // Statistical outlier removal parameters
    struct StatisticalParams {
        int k_neighbors;
        float std_multiplier;
        
        StatisticalParams() : k_neighbors(50), std_multiplier(1.0f) {}
    };
    
    // Radius outlier removal parameters
    struct RadiusParams {
        float radius;
        int min_neighbors;
        
        RadiusParams() : radius(0.1f), min_neighbors(2) {}
    };
    
    // Statistical outlier removal
    static void removeStatisticalOutliers(PointCloud& cloud, 
                                         const StatisticalParams& params = StatisticalParams());
    
    // Radius outlier removal
    static void removeRadiusOutliers(PointCloud& cloud,
                                    const RadiusParams& params = RadiusParams());
    
    // Get indices of outliers without removing them
    static std::vector<size_t> findStatisticalOutliers(const PointCloud& cloud,
                                                       const StatisticalParams& params = StatisticalParams());
    
    static std::vector<size_t> findRadiusOutliers(const PointCloud& cloud,
                                                  const RadiusParams& params = RadiusParams());
    
private:
    // Helper functions
    static std::vector<float> computeNearestNeighborDistances(const PointCloud& cloud, int k);
    static void computeMeanStdDev(const std::vector<float>& values, float& mean, float& stddev);
    static int countNeighborsInRadius(const PointCloud& cloud, size_t point_idx, float radius);
};

} // namespace pcv