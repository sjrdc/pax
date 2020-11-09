enable_testing()
find_package(GTest REQUIRED)

add_executable(testpx testpx.cpp testmain.cpp)
target_link_libraries(testpx GTest::GTest GTest::Main)

add_test(AllTestsInFoo testpx)
