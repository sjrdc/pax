
cmake_minimum_required(VERSION 3.14)
project(px)

set(CMAKE_CXX_STANDARD 20) 
set(CXX_STANDARD_REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(px INTERFACE)
target_include_directories(px INTERFACE 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/pxConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS px
    EXPORT pxTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/pxConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/pxConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mylib
)

install(EXPORT pxTargets DESTINATION lib/cmake/px)
install(FILES "${PROJECT_BINARY_DIR}/pxConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/pxConfig.cmake"
        DESTINATION lib/cmake/px)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

add_executable(example example/example.cpp)
if (UNIX AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
   target_link_libraries(example stdc++fs)
endif()
target_link_libraries(example px)

add_subdirectory(test)
