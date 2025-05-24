#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include "save_data_in_csv.h"
#include <future>
#include <atomic>

using namespace std;

// This will be set by user input
int MAX_THREADS = thread::hardware_concurrency();
int THREAD_THRESHOLD = 1000;

// Global atomic counter for running tasks
std::atomic<int> running_tasks(0);

// Store baseline time for accurate speedup calculations
double get_baseline_time() {
    ifstream file("baseline_time.txt");
    double baseline = 0.0;
    
    if (file.is_open()) {
        file >> baseline;
        file.close();
    }
    
    return baseline;
}

void save_baseline_time(double time) {
    ofstream file("baseline_time.txt");
    
    if (file.is_open()) {
        file << time;
        file.close();
        cout << "Baseline time saved: " << time << " ms" << endl;
    }
}

int number_threads(){
    return thread::hardware_concurrency();
}

void quicksort(vector<int>& arr, int begin, int end, int depth = 0) {
    if (begin >= end) return;

    // نختار عنصر في النص كـ pivot
    int pivot = arr[begin + (end - begin) / 2];

    // نقسم العناصر حول الـ pivot
    int left = begin;
    int right = end;

    while (left <= right) {
        while (arr[left] < pivot) left++;
        while (arr[right] > pivot) right--;

        if (left <= right) {
            swap(arr[left], arr[right]);
            left++;
            right--;
        }
    }

    // نعمل خيوط فرعية لو الحجم كبير وما زال عندنا خيوط متاحة
    thread t1, t2;
    bool use_thread1 = (right - begin > THREAD_THRESHOLD) && (depth < MAX_THREADS - 1);
    bool use_thread2 = (end - left > THREAD_THRESHOLD) && (depth < MAX_THREADS - 1);

    if (use_thread1) {
        t1 = thread(quicksort, ref(arr), begin, right, depth + 1);
    } else {
        quicksort(arr, begin, right, depth);
    }

    if (use_thread2) {
        t2 = thread(quicksort, ref(arr), left, end, depth + 1);
    } else {
        quicksort(arr, left, end, depth);
    }

    // لو شغّلنا threads نستناهم يخلصوا
    if (use_thread1) t1.join();
    if (use_thread2) t2.join();
}

// Single-threaded version for baseline comparison
void quicksort_single_thread(vector<int>& arr, int begin, int end) {
    if (begin >= end) return;

    int pivot = arr[begin + (end - begin) / 2];
    int left = begin;
    int right = end;

    while (left <= right) {
        while (arr[left] < pivot) left++;
        while (arr[right] > pivot) right--;

        if (left <= right) {
            swap(arr[left], arr[right]);
            left++;
            right--;
        }
    }

    quicksort_single_thread(arr, begin, right);
    quicksort_single_thread(arr, left, end);
}

vector<int> generate_random_array(int size) {
    vector<int> data(size);
    
    // Use random device as seed for random generator
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, size * 10);
    
    // Fill array with random values
    for (int i = 0; i < size; i++) {
        data[i] = distrib(gen);
    }
    
    return data;
}

void quicksort_parallel(vector<int>& arr, int begin, int end, int max_threads) {
    if (begin >= end) return;

    int pivot = arr[begin + (end - begin) / 2];
    int left = begin, right = end;

    while (left <= right) {
        while (arr[left] < pivot) left++;
        while (arr[right] > pivot) right--;
        if (left <= right) {
            swap(arr[left], arr[right]);
            left++; right--;
        }
    }

    std::future<void> fut1, fut2;
    bool spawn1 = false, spawn2 = false;

    if (running_tasks < max_threads) {
        running_tasks++;
        spawn1 = true;
        fut1 = std::async(std::launch::async, quicksort_parallel, std::ref(arr), begin, right, max_threads);
    } else {
        quicksort_parallel(arr, begin, right, max_threads);
    }

    if (running_tasks < max_threads) {
        running_tasks++;
        spawn2 = true;
        fut2 = std::async(std::launch::async, quicksort_parallel, std::ref(arr), left, end, max_threads);
    } else {
        quicksort_parallel(arr, left, end, max_threads);
    }

    if (spawn1) { fut1.get(); running_tasks--; }
    if (spawn2) { fut2.get(); running_tasks--; }
}

// Function to run tests for a specific array size
void run_tests_for_size(int array_size, const string& difficulty) {
    double baseline_time = 0.0;
    int max_available_threads = number_threads();
    
    cout << "\n=== Testing " << difficulty << " workload (Array size: " << array_size << ") ===" << endl;
    cout << "Your system has " << max_available_threads << " hardware threads available." << endl;
    
    // Generate random array once
    vector<int> original_data = generate_random_array(array_size);
    
    // First run with single thread to establish baseline
    vector<int> baseline_data = original_data;
    auto start = chrono::high_resolution_clock::now();
    quicksort_single_thread(baseline_data, 0, baseline_data.size() - 1);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> baseline_duration = end - start;
    baseline_time = baseline_duration.count();
    save_baseline_time(baseline_time);
    cout << "Baseline single-thread time: " << baseline_time << " ms" << endl;
    
    // Loop through all possible thread counts
    for (int thread_count = 1; thread_count <= max_available_threads; thread_count++) {
        cout << "\nTesting with " << thread_count << " thread(s)..." << endl;
        
        // Set the global max threads value
        MAX_THREADS = thread_count;
        
        // Make a copy of the original data for this test
        vector<int> data = original_data;
        
        // Measure multi-threaded sorting time
        running_tasks = 0; // Reset before each run
        start = chrono::high_resolution_clock::now();
        quicksort_parallel(data, 0, data.size() - 1, thread_count);
        end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> multi_threaded_time = end - start;
        
        cout << "Multi-threaded sorting completed in " << multi_threaded_time.count() << " ms" << endl;
        
        // Calculate speedup using the baseline time
        double speedup = baseline_time / multi_threaded_time.count();
        
        cout << "Speedup: " << speedup << "x" << endl;
        
        // Save performance data to CSV with difficulty level in filename
        string filename = "quicksort_performance_" + difficulty + ".csv";
        save_performance_data(array_size, thread_count, multi_threaded_time.count(), speedup, filename);
    }
}

int main() {
    // Test with three different array sizes
    run_tests_for_size(100000, "light");    // Light workload
    run_tests_for_size(1000000, "medium");  // Medium workload
    run_tests_for_size(10000000, "hard");   // Hard workload
    
    return 0;
}