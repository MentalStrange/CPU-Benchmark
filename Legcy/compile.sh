#!/bin/bash

# Compile the C++ code with all necessary flags
clang++ -std=c++14 -stdlib=libc++ -lpthread quick_sort_multithreding.cpp -o quick_sort_multithreding

# Make executable if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful! Run the program with ./quick_sort_multithreding"
    # Make the file executable
    chmod +x quick_sort_multithreding
else
    echo "Compilation failed!"
fi 