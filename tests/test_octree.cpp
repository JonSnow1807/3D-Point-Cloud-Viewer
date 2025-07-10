#include <gtest/gtest.h>
#include "core/PointCloud.h"
#include "core/Octree.h"
#include <random>

using namespace pcv;

class OctreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test point cloud
        cloud = std::make_shared<PointCloud>();
        
        // Add points in a cube pattern
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
                for (int z = 0; z < 10; ++z) {
                    cloud->addPoint(glm::vec3(x, y, z));
                }
            }
        }
    }
    
    PointCloud::Ptr cloud;
};

TEST_F(OctreeTest, BuildOctree) {
    Octree octree(*cloud);
    ASSERT_NO_THROW(octree.build());
    
    // Check that octree was built
    EXPECT_GT(octree.getMaxDepth(), 0);
}

TEST_F(OctreeTest, FrustumQuery) {
    Octree octree(*cloud);
    octree.build();
    
    // Create a simple frustum that includes the origin
    Octree::FrustumPlanes frustum;
    // Near plane at z = -1
    frustum[0] = glm::vec4(0, 0, 1, 1);
    // Far plane at z = 20
    frustum[1] = glm::vec4(0, 0, -1, 20);
    // Left plane
    frustum[2] = glm::vec4(1, 0, 0, 5);
    // Right plane
    frustum[3] = glm::vec4(-1, 0, 0, 5);
    // Bottom plane
    frustum[4] = glm::vec4(0, 1, 0, 5);
    // Top plane
    frustum[5] = glm::vec4(0, -1, 0, 5);
    
    auto results = octree.queryFrustum(frustum);
    
    // Should find some points
    EXPECT_GT(results.size(), 0);
    EXPECT_LT(results.size(), cloud->size());
}

TEST_F(OctreeTest, RadiusQuery) {
    Octree octree(*cloud);
    octree.build();
    
    // Query around center of cloud
    glm::vec3 center(5.0f, 5.0f, 5.0f);
    float radius = 2.0f;
    
    auto results = octree.queryRadius(center, radius);
    
    // Verify all returned points are within radius
    for (size_t idx : results) {
        float dist = glm::length((*cloud)[idx].position - center);
        EXPECT_LE(dist, radius);
    }
    
    // Should find some but not all points
    EXPECT_GT(results.size(), 0);
    EXPECT_LT(results.size(), cloud->size());
}

TEST_F(OctreeTest, BoxQuery) {
    Octree octree(*cloud);
    octree.build();
    
    // Query a box in the middle
    glm::vec3 min_bound(3.0f, 3.0f, 3.0f);
    glm::vec3 max_bound(7.0f, 7.0f, 7.0f);
    
    auto results = octree.queryBox(min_bound, max_bound);
    
    // Verify all returned points are within box
    for (size_t idx : results) {
        const auto& pos = (*cloud)[idx].position;
        EXPECT_GE(pos.x, min_bound.x);
        EXPECT_LE(pos.x, max_bound.x);
        EXPECT_GE(pos.y, min_bound.y);
        EXPECT_LE(pos.y, max_bound.y);
        EXPECT_GE(pos.z, min_bound.z);
        EXPECT_LE(pos.z, max_bound.z);
    }
    
    // Expected: 5x5x5 = 125 points
    EXPECT_EQ(results.size(), 125);
}

TEST_F(OctreeTest, EmptyCloudHandling) {
    PointCloud empty_cloud;
    Octree octree(empty_cloud);
    
    ASSERT_NO_THROW(octree.build());
    
    // Queries should return empty results
    Octree::FrustumPlanes frustum;
    auto results = octree.queryFrustum(frustum);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(OctreeTest, LargePointCloud) {
    // Test with larger point cloud
    auto large_cloud = std::make_shared<PointCloud>();
    large_cloud->reserve(100000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
    
    for (int i = 0; i < 100000; ++i) {
        large_cloud->addPoint(glm::vec3(dis(gen), dis(gen), dis(gen)));
    }
    
    Octree octree(*large_cloud);
    auto start = std::chrono::high_resolution_clock::now();
    octree.build();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should build in reasonable time (< 1 second for 100k points)
    EXPECT_LT(duration.count(), 1000);
    
    // Should have reasonable depth
    EXPECT_GE(octree.getMaxDepth(), 5);
    EXPECT_LE(octree.getMaxDepth(), 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}