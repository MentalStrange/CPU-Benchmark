# Compile the C++ code for the parallel split sort module
clang++ -std=c++14 -stdlib=libc++ -lpthread main_parallel_split.cpp parallel_split_sort.cpp -o parallel_split_sort

# Make executable if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Run the program with ./parallel_split_sort"
    chmod +x parallel_split_sort
else
    echo "Compilation failed!"
fi 