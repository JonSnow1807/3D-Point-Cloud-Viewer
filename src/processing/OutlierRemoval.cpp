#include "processing/OutlierRemoval.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <glm/geometric.hpp>

namespace pcv {

void OutlierRemoval::removeStatisticalOutliers(PointCloud& cloud, const StatisticalParams& params) {
    auto outlier_indices = findStatisticalOutliers(cloud, params);
    
    // Sort indices in descending order for safe removal
    std::sort(outlier_indices.begin(), outlier_indices.end(), std::greater<size_t>());
    
    // Remove outliers
    auto& points = cloud.getPoints();
    for (size_t idx : outlier_indices) {
        points.erase(points.begin() + idx);
    }
}

void OutlierRemoval::removeRadiusOutliers(PointCloud& cloud, const RadiusParams& params) {
    auto outlier_indices = findRadiusOutliers(cloud, params);
    
    // Sort indices in descending order for safe removal
    std::sort(outlier_indices.begin(), outlier_indices.end(), std::greater<size_t>());
    
    // Remove outliers
    auto& points = cloud.getPoints();
    for (size_t idx : outlier_indices) {
        points.erase(points.begin() + idx);
    }
}

std::vector<size_t> OutlierRemoval::findStatisticalOutliers(const PointCloud& cloud, 
                                                            const StatisticalParams& params) {
    std::vector<size_t> outliers;
    
    // Compute nearest neighbor distances for all points
    auto distances = computeNearestNeighborDistances(cloud, params.k_neighbors);
    
    // Compute mean and standard deviation
    float mean, stddev;
    computeMeanStdDev(distances, mean, stddev);
    
    // Threshold for outlier detection
    float threshold = mean + params.std_multiplier * stddev;
    
    // Find outliers
    for (size_t i = 0; i < distances.size(); ++i) {
        if (distances[i] > threshold) {
            outliers.push_back(i);
        }
    }
    
    return outliers;
}

std::vector<size_t> OutlierRemoval::findRadiusOutliers(const PointCloud& cloud,
                                                       const RadiusParams& params) {
    std::vector<size_t> outliers;
    
    for (size_t i = 0; i < cloud.size(); ++i) {
        int neighbor_count = countNeighborsInRadius(cloud, i, params.radius);
        
        if (neighbor_count < params.min_neighbors) {
            outliers.push_back(i);
        }
    }
    
    return outliers;
}

std::vector<float> OutlierRemoval::computeNearestNeighborDistances(const PointCloud& cloud, int k) {
    std::vector<float> avg_distances(cloud.size(), 0.0f);
    
    // For each point
    for (size_t i = 0; i < cloud.size(); ++i) {
        const glm::vec3& point = cloud[i].position;
        std::vector<float> distances;
        
        // Compute distances to all other points
        for (size_t j = 0; j < cloud.size(); ++j) {
            if (i != j) {
                float dist = glm::distance(point, cloud[j].position);
                distances.push_back(dist);
            }
        }
        
        // Sort distances and take k nearest
        std::sort(distances.begin(), distances.end());
        
        // Compute average of k nearest neighbors
        float sum = 0.0f;
        int count = std::min(k, static_cast<int>(distances.size()));
        for (int j = 0; j < count; ++j) {
            sum += distances[j];
        }
        
        avg_distances[i] = count > 0 ? sum / count : 0.0f;
    }
    
    return avg_distances;
}

void OutlierRemoval::computeMeanStdDev(const std::vector<float>& values, float& mean, float& stddev) {
    if (values.empty()) {
        mean = 0.0f;
        stddev = 0.0f;
        return;
    }
    
    // Compute mean
    mean = std::accumulate(values.begin(), values.end(), 0.0f) / values.size();
    
    // Compute standard deviation
    float variance = 0.0f;
    for (float value : values) {
        float diff = value - mean;
        variance += diff * diff;
    }
    variance /= values.size();
    stddev = std::sqrt(variance);
}

int OutlierRemoval::countNeighborsInRadius(const PointCloud& cloud, size_t point_idx, float radius) {
    const glm::vec3& point = cloud[point_idx].position;
    float radius_sq = radius * radius;
    int count = 0;
    
    for (size_t i = 0; i < cloud.size(); ++i) {
        if (i != point_idx) {
            float dist = glm::distance(point, cloud[i].position);
            float dist_sq = dist * dist;
            if (dist_sq <= radius_sq) {
                count++;
            }
        }
    }
    
    return count;
}

} // namespace pcv