@echo off
REM Windows compilation script for quick_sort_multithreding

REM First, copy the Windows version of the header file to be used
copy /Y save_data_in_csv_windows.h save_data_in_csv.h

REM Try using g++ (MinGW) if available
where g++ >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Compiling with g++...
    g++ -std=c++14 -pthread quick_sort_multithreding_windows.cpp -o quick_sort_multithreding.exe
) else (
    REM Try using MSVC cl compiler if available
    where cl >nul 2>nul
    if %ERRORLEVEL% == 0 (
        echo Compiling with MSVC...
        cl /EHsc /std:c++14 quick_sort_multithreding_windows.cpp /Fe:quick_sort_multithreding.exe
    ) else (
        echo No suitable C++ compiler found.
        echo Please install MinGW (g++) or Visual Studio (cl).
        exit /b 1
    )
)

REM Check if compilation was successful
if %ERRORLEVEL% == 0 (
    echo Compilation successful! Run the program with quick_sort_multithreding.exe
) else (
    echo Compilation failed!
)

pause 