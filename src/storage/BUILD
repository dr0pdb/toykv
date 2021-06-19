load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "storage_library",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common_library",
        "//src/storage/buffer:storage_buffer",
        "//src/storage/disk:storage_disk",
        "//src/storage/index:storage_index",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@glog",
    ],
)
