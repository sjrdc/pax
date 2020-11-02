enable_testing()
find_package(GTest REQUIRED)

add_executable(testpax main.cpp)
target_link_libraries(testpax GTest::GTest GTest::Main)

add_test(AllTestsInFoo testpax)
