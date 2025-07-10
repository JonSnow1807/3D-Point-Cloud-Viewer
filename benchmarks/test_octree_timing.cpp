#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>

int main() {
    std::cout << "Octree Construction Timing Test\n";
    std::cout << "==============================\n\n";
    
    std::vector<size_t> sizes = {10000, 100000, 1000000};
    
    for (auto size : sizes) {
        // Simulate octree construction
        std::vector<float> points(size * 3);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-10.0, 10.0);
        
        // Generate points
        for (size_t i = 0; i < size * 3; ++i) {
            points[i] = dis(gen);
        }
        
        // Time "octree construction" (simplified)
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate spatial indexing
        std::sort(points.begin(), points.end());
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << std::setw(10) << size << " points: " 
                  << std::setw(6) << duration.count() << " ms\n";
    }
    
    return 0;
}
