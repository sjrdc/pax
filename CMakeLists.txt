
cmake_minimum_required(VERSION 3.28)
project(px)

set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
cmake_minimum_required(VERSION 3.28)
project(std_module_example CXX)

# Turning off extensions avoids and issue with the clang 16 compiler
# clang 17 and greater can avoid this setting
set(CMAKE_CXX_EXTENSIONS OFF)

# Create a library
add_library(px)
# Add the module file to the library
target_sources(px
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/px.cpp
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/pxConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)


add_executable(example example/example.cpp)
if (UNIX AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
   target_link_libraries(example stdc++fs)
endif()
target_link_libraries(example px)

add_subdirectory(test)
