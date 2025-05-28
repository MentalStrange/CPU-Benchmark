#include "optimized_parallel_sort.h"
#include <vector>
#include <thread>
#include <future>
#include <algorithm>
#include <memory>
#include <queue>
#include <cstring>
#ifdef __linux__
#include <numa.h>
#endif
#include <sched.h>
#include <iostream>
#include <cmath>

// Optimized in-place quicksort with better pivot selection
void optimized_quicksort(std::vector<long>::iterator begin, std::vector<long>::iterator end) {
    if (std::distance(begin, end) <= 1) return;
    
    // Use median-of-three pivot selection for better performance
    auto mid = begin + std::distance(begin, end) / 2;
    if (*mid < *begin) std::iter_swap(begin, mid);
    if (*(end-1) < *begin) std::iter_swap(begin, end-1);
    if (*(end-1) < *mid) std::iter_swap(mid, end-1);
    
    auto pivot = *(end-1);
    auto partition_point = std::partition(begin, end-1, [pivot](const long& x) { return x < pivot; });
    std::iter_swap(partition_point, end-1);
    
    // Use iterative approach for larger partition, recursive for smaller (tail recursion optimization)
    auto left_size = std::distance(begin, partition_point);
    auto right_size = std::distance(partition_point + 1, end);
    
    if (left_size > right_size) {
        optimized_quicksort(partition_point + 1, end);
        end = partition_point;
        if (std::distance(begin, end) > 1) optimized_quicksort(begin, end);
    } else {
        optimized_quicksort(begin, partition_point);
        begin = partition_point + 1;
        if (std::distance(begin, end) > 1) optimized_quicksort(begin, end);
    }
}

// Hybrid sort that switches to introsort for small arrays
void hybrid_quicksort(std::vector<long>::iterator begin, std::vector<long>::iterator end, int depth_limit) {
    const size_t INSERTION_SORT_THRESHOLD = 16;
    size_t size = std::distance(begin, end);
    
    if (size <= INSERTION_SORT_THRESHOLD) {
        // Use insertion sort for small arrays
        for (auto it = begin + 1; it != end; ++it) {
            auto key = *it;
            auto j = it - 1;
            while (j >= begin && *j > key) {
                *(j + 1) = *j;
                --j;
            }
            *(j + 1) = key;
        }
        return;
    }
    
    if (depth_limit == 0) {
        // Fall back to heapsort to avoid worst-case O(n²)
        std::make_heap(begin, end);
        std::sort_heap(begin, end);
        return;
    }
    
    // Standard quicksort with median-of-three
    auto mid = begin + size / 2;
    if (*mid < *begin) std::iter_swap(begin, mid);
    if (*(end-1) < *begin) std::iter_swap(begin, end-1);
    if (*(end-1) < *mid) std::iter_swap(mid, end-1);
    
    auto pivot = *(end-1);
    auto partition_point = std::partition(begin, end-1, [pivot](const long& x) { return x < pivot; });
    std::iter_swap(partition_point, end-1);
    
    hybrid_quicksort(begin, partition_point, depth_limit - 1);
    hybrid_quicksort(partition_point + 1, end, depth_limit - 1);
}

// Efficient in-place merge using rotation
void inplace_merge_optimized(std::vector<long>& arr, size_t start, size_t mid, size_t end) {
    if (start >= mid || mid >= end) return;
    
    // If already sorted, no need to merge
    if (arr[mid-1] <= arr[mid]) return;
    
    std::inplace_merge(arr.begin() + start, arr.begin() + mid, arr.begin() + end);
}

// Main optimized parallel sort function
void optimized_parallel_sort(std::vector<long>& arr, int num_threads) {
    if (arr.size() <= 1) return;
    
    const size_t MIN_PARALLEL_SIZE = 10000; // Don't parallelize tiny arrays
    if (arr.size() < MIN_PARALLEL_SIZE || num_threads <= 1) {
        int depth_limit = 2 * std::log2(arr.size());
        hybrid_quicksort(arr.begin(), arr.end(), depth_limit);
        return;
    }
    
    // Calculate optimal chunk sizes based on CPU cache and data size
    size_t optimal_chunks = num_threads * 2; // Use more chunks than threads for better load balancing
    optimal_chunks = std::min(optimal_chunks, arr.size() / MIN_PARALLEL_SIZE);
    optimal_chunks = std::max(1UL, optimal_chunks);
    
    std::vector<std::future<void>> futures;
    size_t chunk_size = arr.size() / optimal_chunks;
    
    // Phase 1: Parallel sort of chunks using all available threads
    for (size_t i = 0; i < optimal_chunks; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == optimal_chunks - 1) ? arr.size() : (i + 1) * chunk_size;
        
        futures.push_back(std::async(std::launch::async, [&arr, start, end]() {
            int depth_limit = 2 * std::log2(end - start);
            hybrid_quicksort(arr.begin() + start, arr.begin() + end, depth_limit);
        }));
    }
    
    // Wait for all sorting tasks to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    // Phase 2: Hierarchical merging with all threads
    std::vector<std::pair<size_t, size_t>> ranges;
    for (size_t i = 0; i < optimal_chunks; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == optimal_chunks - 1) ? arr.size() : (i + 1) * chunk_size;
        ranges.push_back({start, end});
    }
    
    // Merge in a tree-like fashion for better cache locality
    while (ranges.size() > 1) {
        std::vector<std::pair<size_t, size_t>> next_ranges;
        futures.clear();
        
        // Use all available threads for merging
        for (size_t i = 0; i < ranges.size(); i += 2) {
            if (i + 1 < ranges.size()) {
                size_t start = ranges[i].first;
                size_t mid = ranges[i].second;
                size_t end = ranges[i + 1].second;
                
                futures.push_back(std::async(std::launch::async, [&arr, start, mid, end]() {
                    inplace_merge_optimized(arr, start, mid, end);
                }));
                
                next_ranges.push_back({start, end});
            } else {
                next_ranges.push_back(ranges[i]);
            }
        }
        
        // Wait for all merge operations to complete
        for (auto& future : futures) {
            future.wait();
        }
        
        ranges = std::move(next_ranges);
    }
}

// CPU affinity setting for maximum performance
void set_cpu_affinity() {
    #ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    // Set affinity to all available CPUs
    int num_cpus = std::thread::hardware_concurrency();
    for (int i = 0; i < num_cpus; ++i) {
        CPU_SET(i, &cpuset);
    }
    
    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    
    if (result != 0) {
        std::cerr << "Warning: Could not set CPU affinity" << std::endl;
    }
    #endif
}

// NUMA-aware memory allocation (Linux-specific)
std::vector<long> allocate_numa_aware(size_t size) {
    #ifdef __linux__
    if (numa_available() >= 0) {
        numa_set_interleave_mask(numa_all_nodes_ptr);
    }
    #endif
    
    std::vector<long> arr;
    arr.reserve(size);
    return arr;
}