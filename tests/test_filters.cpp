#include <gtest/gtest.h>
#include "core/PointCloud.h"
#include "processing/OutlierRemoval.h"
#include "processing/VoxelDownsampling.h"
#include "processing/Filters.h"
#include <random>

using namespace pcv;

class FilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test point cloud with some outliers
        cloud = std::make_shared<PointCloud>();
        
        // Add regular grid of points
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
                for (int z = 0; z < 10; ++z) {
                    cloud->addPoint(glm::vec3(x * 0.1f, y * 0.1f, z * 0.1f));
                }
            }
        }
        
        // Add some outliers far from the main cloud
        cloud->addPoint(glm::vec3(10.0f, 10.0f, 10.0f));
        cloud->addPoint(glm::vec3(-10.0f, -10.0f, -10.0f));
        cloud->addPoint(glm::vec3(5.0f, 5.0f, 5.0f));
    }
    
    PointCloud::Ptr cloud;
};

TEST_F(FilterTest, VoxelDownsampling) {
    size_t original_size = cloud->size();
    
    VoxelDownsampling::Parameters params;
    params.leaf_size = 0.2f; // Should combine 2x2x2 voxels
    
    auto stats = VoxelDownsampling::getStatistics(*cloud, params);
    EXPECT_LT(stats.downsampled_points, original_size);
    EXPECT_GT(stats.compression_ratio, 0.0f);
    EXPECT_LE(stats.compression_ratio, 1.0f);
    
    // Apply downsampling
    VoxelDownsampling::downsample(*cloud, params);
    EXPECT_EQ(cloud->size(), stats.downsampled_points);
}

TEST_F(FilterTest, StatisticalOutlierRemoval) {
    size_t original_size = cloud->size();
    
    OutlierRemoval::StatisticalParams params;
    params.k_neighbors = 50;
    params.std_multiplier = 1.0f;
    
    auto outliers = OutlierRemoval::findStatisticalOutliers(*cloud, params);
    EXPECT_GT(outliers.size(), 0); // Should find some outliers
    EXPECT_LT(outliers.size(), original_size / 2); // But not too many
    
    // Apply removal
    OutlierRemoval::removeStatisticalOutliers(*cloud, params);
    EXPECT_LT(cloud->size(), original_size);
}

TEST_F(FilterTest, RadiusOutlierRemoval) {
    size_t original_size = cloud->size();
    
    OutlierRemoval::RadiusParams params;
    params.radius = 0.5f;
    params.min_neighbors = 10;
    
    auto outliers = OutlierRemoval::findRadiusOutliers(*cloud, params);
    EXPECT_GT(outliers.size(), 0); // Should find the far outliers
    
    // Apply removal
    OutlierRemoval::removeRadiusOutliers(*cloud, params);
    EXPECT_LT(cloud->size(), original_size);
}

TEST_F(FilterTest, FilterPipeline) {
    size_t original_size = cloud->size();
    
    FilterPipeline pipeline;
    pipeline.addVoxelDownsampling(0.2f)
            .addStatisticalOutlierRemoval(20, 2.0f);
    
    // Apply pipeline
    auto filtered = pipeline.apply(*cloud);
    
    // Should be smaller after both filters
    EXPECT_LT(filtered->size(), original_size);
    EXPECT_GT(filtered->size(), 0);
}

TEST_F(FilterTest, EmptyCloudHandling) {
    PointCloud empty_cloud;
    
    // Voxel downsampling
    VoxelDownsampling::Parameters voxel_params;
    VoxelDownsampling::downsample(empty_cloud, voxel_params);
    EXPECT_EQ(empty_cloud.size(), 0);
    
    // Outlier removal
    OutlierRemoval::StatisticalParams outlier_params;
    OutlierRemoval::removeStatisticalOutliers(empty_cloud, outlier_params);
    EXPECT_EQ(empty_cloud.size(), 0);
}

TEST_F(FilterTest, LargeCloudPerformance) {
    // Create large point cloud
    auto large_cloud = std::make_shared<PointCloud>();
    large_cloud->reserve(100000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 10.0f);
    
    for (int i = 0; i < 100000; ++i) {
        large_cloud->addPoint(glm::vec3(dis(gen), dis(gen), dis(gen)));
    }
    
    // Test voxel downsampling performance
    auto start = std::chrono::high_resolution_clock::now();
    VoxelDownsampling::Parameters params;
    params.leaf_size = 0.1f;
    VoxelDownsampling::downsample(*large_cloud, params);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time
    EXPECT_LT(duration.count(), 500); // Less than 500ms
    EXPECT_LT(large_cloud->size(), 100000); // Should have reduced size
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}