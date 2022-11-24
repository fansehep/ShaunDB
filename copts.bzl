
FVER_FLAGS = [
    "-Wall",
    "-Wextra",
    "-Wconversion-null",
    "-Wformat-security",
    "-Woverlength-strings",
    "-Wpointer-arith",
    "-Wundef",
    "-Wunused-local-typedefs",
    "-Wunused-result",
    "-Wvarargs",
    "-Wvla",
    "-Wwrite-strings",
    "-DNOMINMAX",
    "-std=c++20",
    "-fno-exceptions",
    "-ggdb",
    "-g",
    "-O2",
    # "-Werror",
]

# use clang-addresssanitizer
FVER_DEBUG = [
    "-O0",
    "-g",
    "-ggdb",
]


FVER_BENCHMARK_FLAGS = [
    "-O2",
    "-DNDEBUG",
]

FVER_TEST_FLAGS = [
    "-Wno-conversion-null",
    "-Wno-deprecated-declarations",
    "-Wno-missing-declarations",
    "-Wno-sign-compare",
    "-Wno-unused-function",
    "-Wno-unused-parameter",
    "-Wno-unused-result",
]
