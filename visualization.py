import matplotlib.pyplot as plt
# import numpy as np
import csv

# Read data
thread_counts = []
times = []
speedups = []
efficiencies = []
f_amdahls = []

with open('quicksort_performance.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # Skip header row
    for row in reader:
        thread_counts.append(int(row[1]))  # Thread count
        times.append(float(row[2]))        # Time (ms)
        speedups.append(float(row[3]))     # Speedup
        efficiencies.append(float(row[4])) # Efficiency (%)
        f_amdahls.append(float(row[5]))    # F Amdahl

# Create separate plots for each metric

# 1. Time vs Threads
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, times, color='blue', marker='o', linestyle='-', linewidth=2, markersize=8)
plt.xlabel('Number of Threads')
plt.ylabel('Execution Time (ms)')
plt.title('QuickSort Execution Time with Increasing Threads')
plt.xticks(thread_counts)
plt.grid(True)
plt.savefig('quicksort_time.png')
plt.close()

# 2. Speedup vs Threads
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, speedups, color='red', marker='o', linestyle='-', linewidth=2, markersize=8)
plt.xlabel('Number of Threads')
plt.ylabel('Speedup')
plt.title('QuickSort Speedup with Increasing Threads')
plt.xticks(thread_counts)
plt.grid(True)
plt.savefig('quicksort_speedup.png')
plt.close()

# 3. Efficiency vs Threads
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, efficiencies, color='green', marker='o', linestyle='-', linewidth=2, markersize=8)
plt.xlabel('Number of Threads')
plt.ylabel('Efficiency (%)')
plt.title('QuickSort Efficiency with Increasing Threads')
plt.xticks(thread_counts)
plt.grid(True)
plt.savefig('quicksort_efficiency.png')
plt.close()

# 4. F Amdahl vs Threads
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, f_amdahls, color='purple', marker='o', linestyle='-', linewidth=2, markersize=8)
plt.xlabel('Number of Threads')
plt.ylabel('F Amdahl')
plt.title('QuickSort Amdahl\'s Law Factor with Increasing Threads')
plt.xticks(thread_counts)
plt.grid(True)
plt.savefig('quicksort_amdahl.png')
plt.close()

# 5. Combined plot (all metrics)
plt.figure(figsize=(12, 8))
plt.plot(thread_counts, [t/max(times) for t in times], color='blue', marker='o', linestyle='-', linewidth=2, markersize=8, label='Normalized Time')
plt.plot(thread_counts, speedups, color='red', marker='s', linestyle='-', linewidth=2, markersize=8, label='Speedup')
plt.plot(thread_counts, [e/100 for e in efficiencies], color='green', marker='^', linestyle='-', linewidth=2, markersize=8, label='Efficiency')
plt.plot(thread_counts, f_amdahls, color='purple', marker='d', linestyle='-', linewidth=2, markersize=8, label='F Amdahl')
plt.xlabel('Number of Threads')
plt.ylabel('Metric Value')
plt.title('QuickSort Performance Metrics with Increasing Threads')
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.savefig('quicksort_combined.png')
plt.show()  # Show only the last combined plot

print("All visualization images have been saved:"
      "\n- quicksort_time.png"
      "\n- quicksort_speedup.png"
      "\n- quicksort_efficiency.png"
      "\n- quicksort_amdahl.png"
      "\n- quicksort_combined.png")
# This code reads the quicksort_performance.csv file and creates separate visualizations for each metric.