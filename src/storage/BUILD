load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "storage_library",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    copts = ["-fno-exceptions"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common_library",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@glog",
    ],
)
