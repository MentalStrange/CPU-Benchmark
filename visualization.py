import matplotlib.pyplot as plt
import csv

def read_speedup_data(filename):
    threads = []
    speedups = []
    with open(filename, 'r') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # Skip header
        for row in reader:
            threads.append(int(row[1]))      # Thread count
            speedups.append(float(row[3]))   # Speedup
    return threads, speedups

def read_metric_data(filename, col_index):
    threads = []
    values = []
    with open(filename, 'r') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # Skip header
        for row in reader:
            threads.append(int(row[1]))      # Thread count
            values.append(float(row[col_index]))
    return threads, values

# Read data from each file
threads_light, speedup_light = read_speedup_data('quicksort_performance_light.csv')
threads_medium, speedup_medium = read_speedup_data('quicksort_performance_medium.csv')
threads_hard, speedup_hard = read_speedup_data('quicksort_performance_hard.csv')

# Plot all three speedup curves on one plot
plt.figure(figsize=(12, 7))
plt.plot(threads_light, speedup_light, marker='o', label='light load (3x3)', color='#174A6F')
plt.plot(threads_medium, speedup_medium, marker='o', label='medium load (4x4)', color='#F95D6A')
plt.plot(threads_hard, speedup_hard, marker='o', label='heavy load (4x4)', color='#B30059')

plt.title('Final measurements\nSpeedup', fontsize=32, weight='bold', pad=30)
plt.xlabel('Number of Threads', fontsize=16)
plt.ylabel('Speedup', fontsize=16)
plt.legend(fontsize=14, loc='best')
plt.grid(True)
plt.xticks(threads_light)
plt.tight_layout()
plt.savefig('final_speedup_comparison.png')
plt.show()

print("Visualization image has been saved as: final_speedup_comparison.png")

# --- TIME ---
threads_light, time_light = read_metric_data('quicksort_performance_light.csv', 2)
threads_medium, time_medium = read_metric_data('quicksort_performance_medium.csv', 2)
threads_hard, time_hard = read_metric_data('quicksort_performance_hard.csv', 2)

plt.figure(figsize=(12, 7))
plt.plot(threads_light, time_light, marker='o', label='light load (3x3)', color='#174A6F')
plt.plot(threads_medium, time_medium, marker='o', label='medium load (4x4)', color='#F95D6A')
plt.plot(threads_hard, time_hard, marker='o', label='heavy load (4x4)', color='#B30059')
plt.title('Execution Time vs Number of Threads', fontsize=20, weight='bold')
plt.xlabel('Number of Threads', fontsize=16)
plt.ylabel('Execution Time (ms)', fontsize=16)
plt.legend(fontsize=14, loc='best')
plt.grid(True)
plt.xticks(threads_light)
plt.tight_layout()
plt.savefig('final_time_comparison.png')
plt.close()

# --- EFFICIENCY ---
threads_light, eff_light = read_metric_data('quicksort_performance_light.csv', 4)
threads_medium, eff_medium = read_metric_data('quicksort_performance_medium.csv', 4)
threads_hard, eff_hard = read_metric_data('quicksort_performance_hard.csv', 4)

plt.figure(figsize=(12, 7))
plt.plot(threads_light, eff_light, marker='o', label='light load (3x3)', color='#174A6F')
plt.plot(threads_medium, eff_medium, marker='o', label='medium load (4x4)', color='#F95D6A')
plt.plot(threads_hard, eff_hard, marker='o', label='heavy load (4x4)', color='#B30059')
plt.title('Efficiency vs Number of Threads', fontsize=20, weight='bold')
plt.xlabel('Number of Threads', fontsize=16)
plt.ylabel('Efficiency (%)', fontsize=16)
plt.legend(fontsize=14, loc='best')
plt.grid(True)
plt.xticks(threads_light)
plt.tight_layout()
plt.savefig('final_efficiency_comparison.png')
plt.close()

# --- F AMDAHL ---
threads_light, amdahl_light = read_metric_data('quicksort_performance_light.csv', 5)
threads_medium, amdahl_medium = read_metric_data('quicksort_performance_medium.csv', 5)
threads_hard, amdahl_hard = read_metric_data('quicksort_performance_hard.csv', 5)

plt.figure(figsize=(12, 7))
plt.plot(threads_light, amdahl_light, marker='o', label='light load (3x3)', color='#174A6F')
plt.plot(threads_medium, amdahl_medium, marker='o', label='medium load (4x4)', color='#F95D6A')
plt.plot(threads_hard, amdahl_hard, marker='o', label='heavy load (4x4)', color='#B30059')
plt.title('Amdahl\'s Law Factor vs Number of Threads', fontsize=20, weight='bold')
plt.xlabel('Number of Threads', fontsize=16)
plt.ylabel('F Amdahl', fontsize=16)
plt.legend(fontsize=14, loc='best')
plt.grid(True)
plt.xticks(threads_light)
plt.tight_layout()
plt.savefig('final_amdahl_comparison.png')
plt.close()