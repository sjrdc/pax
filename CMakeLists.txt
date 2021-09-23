
cmake_minimum_required(VERSION 3.14)
project(px)

set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS ${CMAKE_CXX_COMPILER_ID})
add_executable(example example/example.cpp)
if (UNIX AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
   target_link_libraries(example stdc++fs)
endif()

add_subdirectory(test)
