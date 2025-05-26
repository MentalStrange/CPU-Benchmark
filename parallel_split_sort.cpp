#include "parallel_split_sort.h"
#include <vector>
#include <thread>
#include <climits>
#include <algorithm>

// Helper function to merge sorted subarrays
void merge_sorted_parts(std::vector<long>& arr, const std::vector<int>& starts, const std::vector<int>& ends) {
    std::vector<long> result;
    std::vector<size_t> indices(starts.size());
    for (size_t i = 0; i < indices.size(); ++i) indices[i] = starts[i];
    
    while (true) {
        long min_val = LONG_MAX;
        int min_idx = -1;
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i] <= ends[i] && arr[indices[i]] < min_val) {
                min_val = arr[indices[i]];
                min_idx = i;
            }
        }
        if (min_idx == -1) break;
        result.push_back(min_val);
        indices[min_idx]++;
    }
    arr = result;
}

// Forward declaration for quicksort_single_thread
void quicksort_single_thread(std::vector<long>& arr, int begin, int end);

// Parallel initial split sort
void parallel_initial_split_sort(std::vector<long>& arr, int num_threads) {
    std::vector<std::thread> threads;
    int n = arr.size();
    int chunk = n / num_threads;
    std::vector<int> starts(num_threads), ends(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        starts[i] = i * chunk;
        ends[i] = (i == num_threads - 1) ? n - 1 : (i + 1) * chunk - 1;
    }

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&arr, &starts, &ends, i] {
            quicksort_single_thread(arr, starts[i], ends[i]);
        });
    }
    for (auto& t : threads) t.join();

    // Merge sorted parts
    merge_sorted_parts(arr, starts, ends);
} 