load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "example",
    srcs = glob(["**/*.cc"]),
    copts = ["-fno-exceptions"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/common:common_library",
        "//src/storage:storage_library",
        "@com_google_absl//absl/strings",
    ],
)
