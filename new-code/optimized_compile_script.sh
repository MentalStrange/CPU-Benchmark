#!/bin/bash

# High-performance compilation script for optimized parallel sort
echo "=== Compiling Optimized Parallel Sort ==="

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS"
    COMPILER="clang++"
    FLAGS="-std=c++17 -O3 -mtune=native -flto -DNDEBUG -funroll-loops -finline-functions -fomit-frame-pointer -pthread -stdlib=libc++"
else
    echo "Detected Linux"
    COMPILER="g++"
    FLAGS="-std=c++17 -O3 -march=native -mtune=native -flto -DNDEBUG -funroll-loops -finline-functions -fomit-frame-pointer -pthread -fopenmp"
fi

echo "Using $COMPILER compiler"
echo "Compilation command:"
echo "$COMPILER $FLAGS optimized_main.cpp optimized_parallel_sort.cpp -o optimized_parallel_sort"

# Compile
$COMPILER $FLAGS optimized_main.cpp optimized_parallel_sort.cpp -o optimized_parallel_sort

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful!"
    chmod +x optimized_parallel_sort
    
    # Display binary info
    echo ""
    echo "=== Binary Information ==="
    ls -lh optimized_parallel_sort
    file optimized_parallel_sort
    
    echo ""
    echo "=== Usage ==="
    echo "Run with default size (100M elements): ./optimized_parallel_sort"
    echo "Run with custom size: ./optimized_parallel_sort [number_of_elements]"
    echo "Example: ./optimized_parallel_sort 1000000000  # 1 billion elements"
    
    # Performance tips
    echo ""
    echo "=== Performance Tips ==="
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "1. For maximum performance on macOS:"
        echo "   - Close other applications"
        echo "   - Disable Time Machine backups"
        echo "   - Use Activity Monitor to check CPU usage"
    else
        echo "1. For maximum performance, run with CPU governor set to 'performance':"
        echo "   sudo cpupower frequency-set -g performance"
        echo ""
        echo "2. Disable CPU frequency scaling:"
        echo "   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor"
    fi
    echo ""
    echo "3. Set process priority (run as root):"
    echo "   nice -n -20 ./optimized_parallel_sort"
    echo ""
    if [[ "$OSTYPE" != "darwin"* ]]; then
        echo "4. For NUMA systems, you can use numactl:"
        echo "   numactl --interleave=all ./optimized_parallel_sort"
    fi
    
else
    echo "❌ Compilation failed!"
    echo ""
    echo "Common issues and solutions:"
    if [[ "$OSTYPE" != "darwin"* ]]; then
        echo "1. Missing libnuma-dev: sudo apt-get install libnuma-dev"
        echo "2. Missing OpenMP: sudo apt-get install libomp-dev"
    fi
    echo "3. For older systems, remove -march=native flag"
fi

echo ""
echo "=== System Information ==="
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "CPU cores: $(sysctl -n hw.ncpu)"
    echo "Memory: $(sysctl -n hw.memsize | awk '{print $0/1024/1024/1024 " GB"}')"
else
    echo "CPU cores: $(nproc)"
    echo "Memory: $(free -h | grep '^Mem:' | awk '{print $2}')"
fi
echo "Compiler version: $($COMPILER --version | head -n1)"