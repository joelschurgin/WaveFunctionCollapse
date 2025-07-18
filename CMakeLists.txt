cmake_minimum_required(VERSION 3.10)
project(WaveFunctionCollapse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# add_executable(WaveFunctionCollapse main.cpp) 
add_executable(main_seq_Q main_seq_Q.cpp) 
add_executable(main_no_parallel main_no_parallel.cpp) 
add_executable(main_omp main_omp.cpp)
add_executable(main_pattern_caching main_pattern_caching.cpp) 
add_executable(main_pattern_weight main_pattern_weight.cpp)
