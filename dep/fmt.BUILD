package(default_visibility=["//visibility:public"])

cc_library(
    name = "fmt",
    hdrs = glob([
      "include/fmt/*.h",
    ]),
    srcs = glob([
      "src/*.cc",
    ]),
    includes = [
        "include",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)