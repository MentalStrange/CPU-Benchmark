#pragma once
#include <vector>

void parallel_initial_split_sort(std::vector<long>& arr, int num_threads);
void merge_sorted_parts(std::vector<long>& arr, const std::vector<int>& starts, const std::vector<int>& ends); 