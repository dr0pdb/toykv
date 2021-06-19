load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "com_google_googletest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.8.1",
)

git_repository(
    name = "glog",
    remote = "https://github.com/google/glog.git",
    tag = "v0.5.0",
)

# ABSL cpp library lts_2021_03_24
http_archive(
    name = "com_google_absl",
    patch_args = [
        "-p1",
    ],
    strip_prefix = "abseil-cpp-20210324.2",
    urls = [
        "https://github.com/abseil/abseil-cpp/archive/refs/tags/20210324.2.tar.gz",
    ],
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-262ebec3c2296296526740db4aefce68c80de7fa",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/262ebec3c2296296526740db4aefce68c80de7fa.zip"],
)

git_repository(
    name = "com_google_benchmark",
    remote = "https://github.com/google/benchmark.git",
    tag = "v1.5.4",
)

new_local_repository(
    name = "usr_local",
    build_file = "third_party/usr_local.BUILD",
    path = "/usr/local",
)

git_repository(
    name = "com_github_gflags_gflags",
    remote = "https://github.com/gflags/gflags.git",
    tag = "v2.2.2",
)
