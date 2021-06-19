load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "storage_disk",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common_library",
        "@com_google_absl//absl/strings",
    ],
)
