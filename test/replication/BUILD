load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "replication_tests",
    srcs = glob(["**/*.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//src/replication:replication_library",
        "@com_google_googletest//:gtest_main",
    ],
)
