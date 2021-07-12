enable_testing()

add_executable(testpx testpx.cpp testmain.cpp)
target_link_libraries(testpx gtest_main)

include(GoogleTest)
gtest_discover_tests(testpx)
