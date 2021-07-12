
cmake_minimum_required(VERSION 3.14)
project(px)

set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
  )

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_SOURCE_DIR}/include)

message(STATUS ${CMAKE_CXX_COMPILER_ID})
add_executable(example example/example.cpp)
if (UNIX AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
   target_link_libraries(example stdc++fs)
endif()
 
add_subdirectory(test)
