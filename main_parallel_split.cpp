#include "parallel_split_sort.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <cstdlib>
using namespace std;

// Standalone quicksort for use by the parallel split sort
void quicksort_single_thread(std::vector<long>& arr, int begin, int end) {
    if (begin >= end) return;
    long pivot = arr[begin + (end - begin) / 2];
    int left = begin, right = end;
    while (left <= right) {
        while (arr[left] < pivot) left++;
        while (arr[right] > pivot) right--;
        if (left <= right) {
            std::swap(arr[left], arr[right]);
            left++; right--;
        }
    }
    quicksort_single_thread(arr, begin, right);
    quicksort_single_thread(arr, left, end);
}

std::vector<long> generate_random_array(long size) {
    std::vector<long> data(size);
    for (long i = 0; i < size; i++) data[i] = rand();
    return data;
}

int main() {
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads < 1) num_threads = 2;
    long array_size = 1000000000;
    vector<long> arr = generate_random_array(array_size);

    auto start = chrono::high_resolution_clock::now();
    parallel_initial_split_sort(arr, num_threads);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;

    cout << "Parallel split sort with " << num_threads << " threads took: " << duration.count() << " ms" << endl;
    cout << "Sorted: " << (is_sorted(arr.begin(), arr.end()) ? "YES" : "NO") << endl;
    return 0;
} 