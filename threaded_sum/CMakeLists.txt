cmake_minimum_required (VERSION 3.0)
project (parallel_sum)

include_directories(
    ~/vcpkg/installed/x64-linux/include)

add_executable(parallel_sum main.cpp)