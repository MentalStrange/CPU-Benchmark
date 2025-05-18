using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <vector>



double calculate_f_amdahl(double speedup, int thread_count){
    // Calculate Amdahl's law
    // Amdahl's law states that the speedup of a task using multiple processors is limited by the sequential portion of the task.
    //f = p * (1-(1/speedup)) / (p-1)
    // where p is the number of processors and speedup_e is the speedup achieved by using p processors.
    
    // Avoid division by zero if thread_count is 1
    if (thread_count <= 1) {
        return 0.0; // No parallelization with 1 thread
    }
    
    double f = thread_count * (1 - (1 / speedup)) / (thread_count - 1);
    return f;
}

double calculate_efficiency(double speedup, int thread_count){
    // Calculate efficiency
    if (thread_count <= 0) {
        return 0.0;
    }
    double efficiency = speedup / thread_count;
    return efficiency * 100; // Convert to percentage
}

// Function to save sorting performance metrics to CSV file
void save_performance_data(int array_size, int thread_count, double time_ms, double speedup)
{
    // file pointer
    fstream fout;

    // get the efficiency
    double efficiency = calculate_efficiency(speedup, thread_count);
    double f_amdahl = calculate_f_amdahl(speedup, thread_count);

    // Check if file exists to write headers if it's a new file
    bool file_exists = false;
    ifstream file_check("quicksort_performance.csv");
    if (file_check.good()) {
        file_exists = true;
    }
    file_check.close();

    // opens an existing csv file or creates a new file.
    fout.open("quicksort_performance.csv", ios::out | ios::app);
    
    // Write headers if it's a new file
    if (!file_exists) {
        fout << "Array Size,Thread Count,Time (ms),Speedup,Efficiency (%),F Amdahl\n";
    }

    // Write the performance data
    fout << array_size << ","
         << thread_count << ","
         << time_ms << ","
         << speedup << ","
         << efficiency << ","
         << f_amdahl << "\n";
    
    fout.close();
    
    cout << "Performance data saved to quicksort_performance.csv" << endl;
}