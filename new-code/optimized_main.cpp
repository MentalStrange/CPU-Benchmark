#include "optimized_parallel_sort.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <cstdlib>
#include <random>
#ifdef __linux__
#include <numa.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// High-performance random number generation
std::vector<long> generate_random_array(size_t size) {
    std::vector<long> data;
    data.reserve(size);
    
    // Use multiple threads for generation on large arrays
    const size_t chunk_size = size / std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<std::vector<long>> thread_data(std::thread::hardware_concurrency());
    
    for (size_t t = 0; t < std::thread::hardware_concurrency(); ++t) {
        threads.emplace_back([&, t]() {
            size_t start = t * chunk_size;
            size_t end = (t == std::thread::hardware_concurrency() - 1) ? size : (t + 1) * chunk_size;
            
            std::random_device rd;
            std::mt19937_64 gen(rd() + t); // Different seed per thread
            std::uniform_int_distribution<long> dis(0, LONG_MAX);
            
            thread_data[t].reserve(end - start);
            for (size_t i = start; i < end; ++i) {
                thread_data[t].push_back(dis(gen));
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    // Combine results
    for (const auto& chunk : thread_data) {
        data.insert(data.end(), chunk.begin(), chunk.end());
    }
    
    return data;
}

void benchmark_sorting_algorithms(size_t array_size) {
    cout << "=== Parallel Sorting Benchmark ===" << endl;
    cout << "Array size: " << array_size << " elements" << endl;
    
    // Get maximum available threads
    int max_threads = std::thread::hardware_concurrency();
    cout << "Using maximum available threads: " << max_threads << endl;
    
    // Generate test data
    vector<long> arr = generate_random_array(array_size);
    vector<long> arr_copy = arr; // For verification
    
    // Time the optimized parallel sort
    auto start = chrono::high_resolution_clock::now();
    optimized_parallel_sort(arr, max_threads);
    auto end = chrono::high_resolution_clock::now();
    
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "\n=== Results ===" << endl;
    cout << "Time: " << duration.count() << " ms" << endl;
    cout << "Sorted correctly: " << (is_sorted(arr.begin(), arr.end()) ? "YES" : "NO") << endl;
    
    // Calculate throughput
    double throughput = (double)array_size / duration.count() * 1000.0; // elements per second
    cout << "Throughput: " << throughput / 1e6 << " M elements/sec" << endl;
    
    // Memory usage estimation
    double memory_mb = (array_size * sizeof(long)) / (1024.0 * 1024.0);
    cout << "Memory used: ~" << memory_mb << " MB" << endl;
}

int main(int argc, char* argv[]) {
    // Set CPU affinity for maximum performance
    set_cpu_affinity();
    
    size_t array_size = 10000000; // Default: 100M elements
    
    if (argc > 1) {
        array_size = std::stoull(argv[1]);
    }
    
    // Ensure minimum reasonable size
    if (array_size < 1000) {
        array_size = 1000;
        cout << "Minimum array size set to 1000" << endl;
    }
    
    try {
        benchmark_sorting_algorithms(array_size);
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}