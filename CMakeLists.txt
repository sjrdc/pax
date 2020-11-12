
cmake_minimum_required(VERSION 3.6)
project(px)
set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)

if (WIN32)
  set(CMAKE_PREFIX_PATH D:\\libraries\\gtest)
  find_package(gtest)
  add_subdirectory(test)
endif()


add_executable(example example.cpp)
if (UNIX)
  target_link_libraries(example stdc++fs)
endif()
