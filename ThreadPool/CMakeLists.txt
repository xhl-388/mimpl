cmake_minimum_required(VERSION 3.15)

project(ThreadPool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

file(GLOB test_sources src/*.cpp src/*.h src/*.hpp)
add_executable(test ${test_sources})

add_subdirectory(lib)

target_link_libraries(test PUBLIC mylib)