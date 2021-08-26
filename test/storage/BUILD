load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "storage_tests",
    srcs = glob(["**/*.cc"]),
    visibility = ["//visibility:public"],
    deps = [
        "//src/storage:storage_library",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)
