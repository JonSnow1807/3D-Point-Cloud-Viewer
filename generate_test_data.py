#!/usr/bin/env python3
import numpy as np
import time

def generate_point_cloud(filename, num_points):
    """Generate a test point cloud file"""
    print(f"Generating {num_points:,} points...", end='', flush=True)
    start = time.time()
    
    # Generate random points
    np.random.seed(42)  # For reproducibility
    
    # Create different shapes for variety
    shape_type = num_points % 3
    
    if shape_type == 0:  # Sphere
        # Random points in a sphere
        theta = np.random.uniform(0, 2*np.pi, num_points)
        phi = np.random.uniform(0, np.pi, num_points)
        r = np.random.uniform(0, 10, num_points)
        
        x = r * np.sin(phi) * np.cos(theta)
        y = r * np.sin(phi) * np.sin(theta)
        z = r * np.cos(phi)
    
    elif shape_type == 1:  # Cube
        # Random points in a cube
        x = np.random.uniform(-10, 10, num_points)
        y = np.random.uniform(-10, 10, num_points)
        z = np.random.uniform(-10, 10, num_points)
    
    else:  # Torus
        # Random points on a torus
        u = np.random.uniform(0, 2*np.pi, num_points)
        v = np.random.uniform(0, 2*np.pi, num_points)
        R, r = 8, 3
        
        x = (R + r * np.cos(v)) * np.cos(u)
        y = (R + r * np.cos(v)) * np.sin(u)
        z = r * np.sin(v)
    
    # Normalize to [-10, 10] range
    x = 20 * (x - x.min()) / (x.max() - x.min()) - 10
    y = 20 * (y - y.min()) / (y.max() - y.min()) - 10
    z = 20 * (z - z.min()) / (z.max() - z.min()) - 10
    
    # Color based on position
    r_color = (x + 10) / 20
    g_color = (y + 10) / 20
    b_color = (z + 10) / 20
    
    # Write to file
    with open(filename, 'w') as f:
        f.write(f"# Point cloud with {num_points} points\n")
        for i in range(num_points):
            f.write(f"{x[i]:.6f} {y[i]:.6f} {z[i]:.6f} {r_color[i]:.3f} {g_color[i]:.3f} {b_color[i]:.3f}\n")
    
    elapsed = time.time() - start
    print(f" done in {elapsed:.2f}s")
    return elapsed

# Test different sizes
sizes = [10_000, 50_000, 100_000, 250_000, 500_000, 1_000_000, 2_000_000]

print("3D Point Cloud Viewer - Test Data Generator")
print("=" * 50)
print()

total_time = 0
for size in sizes:
    filename = f"test_data/test_{size}.xyz"
    elapsed = generate_point_cloud(filename, size)
    total_time += elapsed
    file_size_mb = size * 6 * 4 / (1024 * 1024)  # Approximate
    print(f"  File size: ~{file_size_mb:.1f} MB")

print(f"\nTotal generation time: {total_time:.2f}s")
print("\nTest files created in test_data/")
