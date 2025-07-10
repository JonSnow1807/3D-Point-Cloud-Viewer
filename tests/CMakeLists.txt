# Find Google Test
find_package(GTest REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${GTEST_INCLUDE_DIRS}
)

# Test executables
add_executable(test_octree test_octree.cpp
    ${PROJECT_SOURCE_DIR}/src/core/PointCloud.cpp
    ${PROJECT_SOURCE_DIR}/src/core/Octree.cpp
)

add_executable(test_filters test_filters.cpp
    ${PROJECT_SOURCE_DIR}/src/core/PointCloud.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/OutlierRemoval.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/VoxelDownsampling.cpp
    ${PROJECT_SOURCE_DIR}/src/processing/Filters.cpp
)

add_executable(test_memory_pool test_memory_pool.cpp
    ${PROJECT_SOURCE_DIR}/src/core/MemoryPool.cpp
)

# Link libraries
target_link_libraries(test_octree GTest::GTest GTest::Main pthread)
target_link_libraries(test_filters GTest::GTest GTest::Main pthread)
target_link_libraries(test_memory_pool GTest::GTest GTest::Main pthread)

# Add tests
add_test(NAME OctreeTest COMMAND test_octree)
add_test(NAME FiltersTest COMMAND test_filters)
add_test(NAME MemoryPoolTest COMMAND test_memory_pool)