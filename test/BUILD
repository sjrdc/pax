
cc_test (
    name = "testpx",
    srcs = ["testpx.cpp"],
    timeout = "short",
    deps = [
        "//:px",
        "@gtest//:gtest",
        "@gtest//:gtest_main" # Only if hello_test.cc has no main()
    ],
)