#!/bin/bash

echo "3D Point Cloud Viewer - Performance Benchmark"
echo "============================================="
echo
echo "System Info:"
echo "- OS: $(sw_vers -productName) $(sw_vers -productVersion)"
echo "- CPU: $(sysctl -n machdep.cpu.brand_string)"
echo "- Memory: $(( $(sysctl -n hw.memsize) / 1024 / 1024 / 1024 )) GB"
echo
echo "Instructions:"
echo "1. For each test, the viewer will open"
echo "2. Wait 5-10 seconds for FPS to stabilize"
echo "3. Note the FPS and visible points from the title bar"
echo "4. Try moving around with WASD + Space+Mouse"
echo "5. Press ESC to close and continue to next test"
echo
echo "Press Enter to begin..."
read

cd build

# Test each file
for points in 10000 50000 100000 250000 500000 1000000 2000000; do
    echo
    echo "TEST: ${points} points"
    echo "===================="
    echo "Loading test_data/test_${points}.xyz"
    echo
    
    # Run the viewer
    ./PointCloudViewer ../test_data/test_${points}.xyz
    
    echo "Test completed. Press Enter for next test..."
    read
done

echo
echo "All tests completed!"
echo "Please fill in your results in PERFORMANCE_RESULTS.md"
