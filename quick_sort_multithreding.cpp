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
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

// This will be set by user input
int MAX_THREADS = thread::hardware_concurrency();
int THREAD_THRESHOLD = 1000;

// Global atomic counter for running tasks
std::atomic<int> running_tasks(0);

// Thread pool implementation
class ThreadPool {
private:
    vector<thread> workers;
    queue<function<void()>> tasks;
    
    mutex queue_mutex;
    condition_variable condition;
    bool stop;
    
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                while(true) {
                    function<void()> task;
                    
                    {
                        unique_lock<mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        
                        if(this->stop && this->tasks.empty())
                            return;
                            
                        task = move(this->tasks.front());
                        this->tasks.pop();
                    }
                    
                    task();
                }
            });
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            unique_lock<mutex> lock(queue_mutex);
            if(stop) throw runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~ThreadPool() {
        {
            unique_lock<mutex> lock(queue_mutex);
            stop = true;
        }
        
        condition.notify_all();
        for(thread &worker: workers)
            worker.join();
    }
    
    size_t get_thread_count() const {
        return workers.size();
    }
    
    size_t get_queue_size() {
        unique_lock<mutex> lock(queue_mutex);
        return tasks.size();
    }
};

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

void quicksort(vector<long>& arr, int begin, int end, int depth = 0) {
    if (begin >= end) return;

    // Select random pivot
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(begin, end);
    int pivotIndex = distrib(gen);
    long pivot = arr[pivotIndex];

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

    // Create sub-threads if size is large and threads are available
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

    // Wait for threads if we started any
    if (use_thread1) t1.join();
    if (use_thread2) t2.join();
}

// Single-threaded version for baseline comparison
void quicksort_single_thread(vector<long>& arr, int begin, int end) {
    if (begin >= end) return;

    // Select random pivot
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(begin, end);
    int pivotIndex = distrib(gen);
    long pivot = arr[pivotIndex];

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

vector<long> generate_random_array(long size) {
    vector<long> data(size);
    
    // Use random device as seed for random generator
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<long> distrib(1, size * 10);
    
    // Fill array with random values
    for (long i = 0; i < size; i++) {
        data[i] = distrib(gen);
    }
    
    return data;
}

// New parallel quicksort implementation using thread pool
void quicksort_parallel_worker(vector<long>& arr, int begin, int end, ThreadPool& pool, atomic<int>& pending_tasks) {
    if (begin >= end) {
        pending_tasks--;
        return;
    }

    // Select random pivot
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(begin, end);
    int pivotIndex = distrib(gen);
    long pivot = arr[pivotIndex];

    int left = begin, right = end;

    while (left <= right) {
        while (arr[left] < pivot) left++;
        while (arr[right] > pivot) right--;
        if (left <= right) {
            swap(arr[left], arr[right]);
            left++; right--;
        }
    }

    // Always create tasks for both partitions
    // Let the thread pool handle scheduling
    if (right - begin > THREAD_THRESHOLD) {
        pending_tasks++;
        pool.enqueue([&arr, begin, right, &pool, &pending_tasks] {
            quicksort_parallel_worker(arr, begin, right, pool, pending_tasks);
        });
    } else {
        quicksort_single_thread(arr, begin, right);
    }

    if (end - left > THREAD_THRESHOLD) {
        pending_tasks++;
        pool.enqueue([&arr, left, end, &pool, &pending_tasks] {
            quicksort_parallel_worker(arr, left, end, pool, pending_tasks);
        });
    } else {
        quicksort_single_thread(arr, left, end);
    }
    
    pending_tasks--;
}

void quicksort_parallel(vector<long>& arr, int begin, int end, int max_threads) {
    // Create thread pool with specified number of threads
    ThreadPool pool(max_threads);
    
    // Counter for pending tasks
    atomic<int> pending_tasks(1);
    
    // Start the first task
    pool.enqueue([&arr, begin, end, &pool, &pending_tasks] {
        quicksort_parallel_worker(arr, begin, end, pool, pending_tasks);
    });
    
    // Wait for all tasks to complete
    while (pending_tasks > 0) {
        this_thread::sleep_for(chrono::milliseconds(1));
    }
}

// Function to run tests for a specific array size
void run_tests_for_size(long array_size, const string& difficulty) {
    double baseline_time = 0.0;
    int max_available_threads = number_threads();
    
    cout << "\n=== Testing " << difficulty << " workload (Array size: " << array_size << ") ===" << endl;
    cout << "Your system has " << max_available_threads << " hardware threads available." << endl;
    
    // Generate random array once
    vector<long> original_data = generate_random_array(array_size);
    
    // First run with single thread to establish baseline
    vector<long> baseline_data = original_data;
    auto start = chrono::high_resolution_clock::now();
    quicksort_single_thread(baseline_data, 0, baseline_data.size() - 1);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> baseline_duration = end - start;
    baseline_time = baseline_duration.count();
    save_baseline_time(baseline_time);
    cout << "Baseline single-thread time: " << baseline_time << " ms" << endl;
    
    // Set thread threshold based on array size and available threads
    THREAD_THRESHOLD = max(500, static_cast<int>(array_size / (max_available_threads * 8)));
    cout << "Using thread threshold: " << THREAD_THRESHOLD << " elements" << endl;
    
    // Loop through different thread counts
    for (int thread_count = 1; thread_count <= max_available_threads; thread_count++) {
        cout << "\nTesting with " << thread_count << " thread(s)..." << endl;
        
        // Make a copy of the original data for this test
        vector<long> data = original_data;
        
        // Measure multi-threaded sorting time
        start = chrono::high_resolution_clock::now();
        quicksort_parallel(data, 0, data.size() - 1, thread_count);
        end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> multi_threaded_time = end - start;
        
        // Verify that the array is properly sorted
        bool is_sorted = std::is_sorted(data.begin(), data.end());
        if (!is_sorted) {
            cout << "WARNING: Array is not properly sorted!" << endl;
        }
        
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
    run_tests_for_size(100000, "light");     // Light workload
    run_tests_for_size(1000000, "medium");   // Medium workload
    run_tests_for_size(10000000, "heavy");   // Heavy workload
    
    return 0;
}