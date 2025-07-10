cmake_minimum_required(VERSION 3.14)
project(PointCloudViewer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)

# Handle GLEW - simpler approach for macOS
if(APPLE)
    # Direct specification for Homebrew on Apple Silicon
    set(GLEW_INCLUDE_DIRS "/opt/homebrew/include")
    set(GLEW_LIBRARIES "/opt/homebrew/lib/libGLEW.dylib")
else()
    find_package(GLEW REQUIRED)
    set(GLEW_LIBRARIES ${GLEW_LIBRARIES})
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# Collect source files
file(GLOB_RECURSE SOURCES 
    src/*.cpp
    src/core/*.cpp
    src/rendering/*.cpp
    src/processing/*.cpp
    src/utils/*.cpp
)

# Remove Timer.cpp if it exists (header-only)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/Timer.cpp)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    Threads::Threads
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})


# Installation rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY shaders DESTINATION share/${PROJECT_NAME})