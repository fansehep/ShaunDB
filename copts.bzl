
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
    # "-Werror",
    "-DNOMINMAX",
    "-std=c++20",
    "-O2",
    # "-g0",
    # "-fno-exceptions",
]
#TODO: should -fno-exception but the yaml-cpp don't 

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
