load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "storage_tests",
    srcs = glob(["**/*.cc"]),
    copts = [
        "-fno-exceptions",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common_library",
        "//src/storage:storage_library",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)
