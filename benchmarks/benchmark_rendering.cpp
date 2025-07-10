#include <benchmark/benchmark.h>
#include "core/PointCloud.h"
#include "core/Octree.h"
#include "processing/OutlierRemoval.h"
#include "processing/VoxelDownsampling.h"
#include "processing/Filters.h"
#include <random>

using namespace pcv;

// Helper to generate point clouds of various sizes
PointCloud::Ptr generatePointCloud(size_t num_points) {
    auto cloud = std::make_shared<PointCloud>();
    cloud->reserve(num_points);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
    
    for (size_t i = 0; i < num_points; ++i) {
        glm::vec3 pos(dis(gen), dis(gen), dis(gen));
        glm::vec3 color(
            (pos.x + 10.0f) / 20.0f,
            (pos.y + 10.0f) / 20.0f,
            (pos.z + 10.0f) / 20.0f
        );
        cloud->addPoint(pos, color);
    }
    
    return cloud;
}

// Benchmark octree construction
static void BM_OctreeConstruction(benchmark::State& state) {
    auto cloud = generatePointCloud(state.range(0));
    
    for (auto _ : state) {
        Octree octree(*cloud);
        octree.build();
        benchmark::DoNotOptimize(octree.getMaxDepth());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_OctreeConstruction)->Range(1000, 1000000);

// Benchmark frustum culling
static void BM_FrustumCulling(benchmark::State& state) {
    auto cloud = generatePointCloud(state.range(0));
    Octree octree(*cloud);
    octree.build();
    
    // Create a frustum that captures about half the points
    Octree::FrustumPlanes frustum;
    frustum[0] = glm::vec4(0, 0, 1, 5);    // Near
    frustum[1] = glm::vec4(0, 0, -1, 5);   // Far
    frustum[2] = glm::vec4(1, 0, 0, 5);    // Left
    frustum[3] = glm::vec4(-1, 0, 0, 5);   // Right
    frustum[4] = glm::vec4(0, 1, 0, 5);    // Bottom
    frustum[5] = glm::vec4(0, -1, 0, 5);   // Top
    
    for (auto _ : state) {
        auto results = octree.queryFrustum(frustum);
        benchmark::DoNotOptimize(results.size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_FrustumCulling)->Range(1000, 1000000);

// Benchmark radius query
static void BM_RadiusQuery(benchmark::State& state) {
    auto cloud = generatePointCloud(state.range(0));
    Octree octree(*cloud);
    octree.build();
    
    glm::vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 5.0f;
    
    for (auto _ : state) {
        auto results = octree.queryRadius(center, radius);
        benchmark::DoNotOptimize(results.size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_RadiusQuery)->Range(1000, 1000000);

// Benchmark voxel downsampling
static void BM_VoxelDownsampling(benchmark::State& state) {
    auto original_cloud = generatePointCloud(state.range(0));
    
    VoxelDownsampling::Parameters params;
    params.leaf_size = 0.1f;
    
    for (auto _ : state) {
        auto cloud = std::make_shared<PointCloud>(*original_cloud);
        VoxelDownsampling::downsample(*cloud, params);
        benchmark::DoNotOptimize(cloud->size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_VoxelDownsampling)->Range(1000, 100000);

// Benchmark statistical outlier removal
static void BM_StatisticalOutlierRemoval(benchmark::State& state) {
    auto original_cloud = generatePointCloud(state.range(0));
    
    OutlierRemoval::StatisticalParams params;
    params.k_neighbors = 20;
    params.std_multiplier = 1.0f;
    
    for (auto _ : state) {
        auto cloud = std::make_shared<PointCloud>(*original_cloud);
        OutlierRemoval::removeStatisticalOutliers(*cloud, params);
        benchmark::DoNotOptimize(cloud->size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StatisticalOutlierRemoval)->Range(1000, 10000);

// Benchmark filter pipeline
static void BM_FilterPipeline(benchmark::State& state) {
    auto original_cloud = generatePointCloud(state.range(0));
    
    FilterPipeline pipeline;
    pipeline.addVoxelDownsampling(0.1f)
            .addStatisticalOutlierRemoval(20, 1.0f);
    
    for (auto _ : state) {
        auto filtered = pipeline.apply(*original_cloud);
        benchmark::DoNotOptimize(filtered->size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_FilterPipeline)->Range(1000, 50000);

// Benchmark memory allocation
static void BM_PointCloudAllocation(benchmark::State& state) {
    size_t num_points = state.range(0);
    
    for (auto _ : state) {
        auto cloud = std::make_shared<PointCloud>();
        cloud->reserve(num_points);
        
        for (size_t i = 0; i < num_points; ++i) {
            cloud->addPoint(glm::vec3(i, i, i));
        }
        
        benchmark::DoNotOptimize(cloud->size());
    }
    
    state.SetItemsProcessed(state.iterations() * num_points);
}
BENCHMARK(BM_PointCloudAllocation)->Range(1000, 100000);

// Benchmark LOD query
static void BM_LODQuery(benchmark::State& state) {
    auto cloud = generatePointCloud(state.range(0));
    Octree octree(*cloud);
    octree.build();
    
    glm::vec3 view_position(15.0f, 15.0f, 15.0f);
    Octree::FrustumPlanes frustum;
    // Setup frustum looking at origin
    frustum[0] = glm::vec4(0.707f, 0, 0.707f, 0);  // Near
    frustum[1] = glm::vec4(-0.707f, 0, -0.707f, 30); // Far
    frustum[2] = glm::vec4(0.894f, 0, -0.447f, 0);   // Left
    frustum[3] = glm::vec4(-0.894f, 0, 0.447f, 0);   // Right
    frustum[4] = glm::vec4(0, 0.894f, -0.447f, 0);   // Bottom
    frustum[5] = glm::vec4(0, -0.894f, 0.447f, 0);   // Top
    
    for (auto _ : state) {
        auto results = octree.queryLOD(view_position, frustum, 10.0f);
        benchmark::DoNotOptimize(results.size());
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_LODQuery)->Range(10000, 1000000);

BENCHMARK_MAIN();