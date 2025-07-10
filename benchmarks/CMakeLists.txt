# Find Google Benchmark
find_package(benchmark REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

# Benchmark executable
add_executable(benchmark_rendering 
    benchmark_rendering.cpp
    ${PROJECT_SOURCE_DIR}/src/core/PointCloud.cpp
    ${PROJECT_SOURCE_DIR}/src/core/Octree.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/OutlierRemoval.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/VoxelDownsampling.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/Filters.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/FileIO.cpp
)

# Link libraries
target_link_libraries(benchmark_rendering 
    benchmark::benchmark
    benchmark::benchmark_main
    pthread
)