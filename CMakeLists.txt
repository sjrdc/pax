
cmake_minimum_required(VERSION 3.6)
project(px)
set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)
find_package(gtest)
add_subdirectory(test)

add_executable(example example.cpp)
