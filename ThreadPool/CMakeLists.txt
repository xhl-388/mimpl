cmake_minimum_required(VERSION 3.15)

project(ThreadPool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_effi src/test_effi.cpp)
add_executable(test_func src/test_func.cpp)

add_subdirectory(lib)

target_link_libraries(test_effi PUBLIC mylib pthread)
target_link_libraries(test_func PUBLIC mylib pthread)

find_package(benchmark REQUIRED)
target_link_libraries(test_effi PUBLIC benchmark::benchmark)