#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include "save_data_in_csv.h"

using namespace std;

// This will be set by user input
int MAX_THREADS = thread::hardware_concurrency();
int THREAD_THRESHOLD = 1000;

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

    // Select pivot element from the middle
    int pivot = arr[begin + (end - begin) / 2];

    // Partition the array around the pivot
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

    // Create threads for large sub-arrays if we have thread budget
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

    // Join threads if they were created
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

int main() {
    int array_size;
    int thread_count;
    
    // Display available threads in the system
    cout << "Your system has " << number_threads() << " hardware threads available." << endl;
    
    // Get baseline time for proper speedup calculations
    double baseline_time = get_baseline_time();
    if (baseline_time > 0) {
        cout << "Using baseline single-thread time: " << baseline_time << " ms" << endl;
    }
    
    // Get array size from user
    cout << "Enter array size: ";
    cin >> array_size;
    
    // Get desired thread count from user
    cout << "Enter number of threads to use (1-" << number_threads() << "): ";
    cin >> thread_count;
    
    // Validate thread count
    if (thread_count < 1) {
        thread_count = 1;
        cout << "Thread count set to minimum (1)" << endl;
    } else if (thread_count > number_threads()) {
        thread_count = number_threads();
        cout << "Thread count limited to system maximum (" << number_threads() << ")" << endl;
    }
    
    // Set the global max threads value
    MAX_THREADS = thread_count;
    
    // Generate random array
    vector<int> data = generate_random_array(array_size);
    
    cout << "Array size: " << array_size << endl;
    cout << "Using " << MAX_THREADS << " threads for sorting" << endl;
    
    // Measure multi-threaded sorting time
    auto start = chrono::high_resolution_clock::now();
    quicksort(data, 0, data.size() - 1);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> multi_threaded_time = end - start;
    
    cout << "Multi-threaded sorting completed in " << multi_threaded_time.count() << " ms" << endl;
    
    double speedup;
    double single_threaded_time_ms;
    
    // For thread_count=1, we measure and save as baseline
    if (thread_count == 1 || baseline_time <= 0) {
        // Measure single-threaded time if we don't have a baseline
        vector<int> data_copy = data; // Make a copy for single-threaded version
        // Measure single-threaded sorting time for speedup calculation
        start = chrono::high_resolution_clock::now();
        quicksort_single_thread(data_copy, 0, data_copy.size() - 1);
        end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> measured_time = end - start;
        single_threaded_time_ms = measured_time.count();
        
        cout << "Single-threaded sorting completed in " << single_threaded_time_ms << " ms" << endl;
        
        // If this is thread_count=1, save as baseline
        if (thread_count == 1) {
            baseline_time = multi_threaded_time.count();
            save_baseline_time(baseline_time);
            speedup = 1.0; // By definition, speedup is 1.0 for the baseline
        } else {
            speedup = single_threaded_time_ms / multi_threaded_time.count();
        }
    } else {
        // Use saved baseline time from thread_count=1 run
        single_threaded_time_ms = baseline_time;
        speedup = baseline_time / multi_threaded_time.count();
        cout << "Using baseline single-thread time for speedup calculation" << endl;
    }
    
    cout << "Speedup: " << speedup << "x" << endl;
    
    // Save performance data to CSV
    save_performance_data(array_size, thread_count, multi_threaded_time.count(), speedup);
    
    // Print first and last few elements to verify sorting
    cout << "First few elements: ";
    for (int i = 0; i < min(10, array_size); i++) {
        cout << data[i] << " ";
    }
    cout << endl;
    
    if (array_size > 10) {
        cout << "Last few elements: ";
        for (int i = max(0, array_size - 10); i < array_size; i++) {
            cout << data[i] << " ";
        }
        cout << endl;
    }
    
    // Wait for user input before exiting (for Windows)
    cout << "Press Enter to exit..." << endl;
    cin.ignore();
    cin.get();
    
    return 0;
} 