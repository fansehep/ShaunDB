
FVER_FLAGS = [
    "-Wall",
    "-Wextra",
    "-fexceptions",
    "-ggdb",
    "-g",
    "-O2",
    # "-Werror",
    "-Werror=array-bounds",
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
