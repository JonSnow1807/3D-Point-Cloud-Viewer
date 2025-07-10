# 3D Point Cloud Viewer

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green.svg)](https://www.opengl.org/)
[![CMake](https://img.shields.io/badge/CMake-3.14+-red.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Performance](https://img.shields.io/badge/Performance-858%20FPS-brightgreen.svg)](benchmarks/PERFORMANCE_RESULTS.md)


A high-performance 3D point cloud visualization engine built with C++17 and OpenGL, capable of rendering 1M+ points at 60 FPS.

## Features

- **High Performance Rendering**: Renders millions of points at 60+ FPS using OpenGL
- **Spatial Optimization**: Octree-based view frustum culling and LOD (Level of Detail) rendering
- **Real-time Processing**: Point cloud filtering with <100ms execution time
- **Memory Efficient**: Custom memory pooling reduces memory footprint by 40%
- **Modular Architecture**: Clean, extensible design with CMake build system

## Performance Metrics

- **Rendering**: 1M+ points at 858 FPS (14x target)
- **Filtering**: <100ms for 500K point datasets
- **Memory**: 40% reduction through custom memory pooling
- **Benchmark**: 3-4x performance improvement over PCL viewer baseline


## Architecture

```
├── Core Components
│   ├── PointCloud: Efficient point data structure
│   ├── Octree: Spatial indexing for culling and LOD
│   └── MemoryPool: Custom memory management
├── Rendering Pipeline
│   ├── Renderer: OpenGL-based rendering engine
│   ├── Camera: Interactive 3D camera system
│   └── Shaders: GPU-accelerated point rendering
└── Processing Filters
    ├── OutlierRemoval: Statistical and radius-based filtering
    └── VoxelDownsampling: Point cloud decimation
```

## Quick Start

### Prerequisites
- C++17 compatible compiler
- CMake 3.14+
- OpenGL 3.3+
- GLFW, GLEW, GLM

### Build
```bash
git clone <repository>
cd point-cloud-viewer
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Run
```bash
./PointCloudViewer                    # Generate sample data
./PointCloudViewer cloud.xyz          # Load from file
```

## Usage

### Controls
- **WASD/QE**: Navigate camera
- **Mouse**: Look around (hold Space)
- **Scroll**: Zoom
- **ESC**: Exit

### File Format
Supports XYZ RGB format:
```
# X Y Z R G B
0.0 0.0 0.0 1.0 0.0 0.0
1.0 0.0 0.0 0.0 1.0 0.0
```

## Implementation Details

### Octree Spatial Indexing
- Hierarchical space partitioning for efficient queries
- View frustum culling eliminates non-visible points
- Dynamic LOD based on distance from viewer

### Memory Pooling
- Pre-allocated memory blocks for points
- Reduces allocation overhead
- 40% memory footprint reduction

### Rendering Pipeline
1. Frustum calculation from camera matrices
2. Octree query for visible points
3. LOD selection based on distance
4. GPU buffer updates
5. Point rendering with custom shaders

## Performance Testing

This project includes comprehensive performance benchmarks:

```bash
# Generate test data (10K to 2M points)
python3 benchmarks/scripts/generate_test_data.py

# Run interactive performance tests
./benchmarks/scripts/run_performance_test.sh

# Measure octree construction time
cd build && make octree_timing
./benchmarks/octree_timing
```
## Future Enhancements

- [ ] Multi-threaded octree construction
- [ ] GPU-based frustum culling
- [ ] Point cloud compression
- [ ] Support for LAS/LAZ formats
- [ ] Real-time point cloud streaming

## License

MIT License - See LICENSE file for details
