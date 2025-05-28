#pragma once

#include <vector>
#include <iterator>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>

// Main optimized parallel sorting function
void optimized_parallel_sort(std::vector<long>& arr, int num_threads);

// Individual sorting algorithms
void optimized_quicksort(std::vector<long>::iterator begin, std::vector<long>::iterator end);
void hybrid_quicksort(std::vector<long>::iterator begin, std::vector<long>::iterator end, int depth_limit);

// Merge operations
void inplace_merge_optimized(std::vector<long>& arr, size_t start, size_t mid, size_t end);

// Performance optimization functions
void set_cpu_affinity();
std::vector<long> allocate_numa_aware(size_t size);

// Work-stealing thread pool for dynamic load balancing
class WorkStealingPool {
private:
    std::vector<std::thread> workers;
    std::vector<std::queue<std::function<void()>>> task_queues;
    std::vector<std::mutex> queue_mutexes;
    std::atomic<bool> stop{false};
    
public:
    WorkStealingPool(size_t num_threads);
    void submit(size_t thread_id, std::function<void()> task);
    ~WorkStealingPool();
};