load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "tests",
    srcs = glob(["**/*.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//test/replication:replication_tests",
        "//test/storage:storage_tests",
        "@com_google_googletest//:gtest_main",
    ],
)
