
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
    # "-g0",
    "-O2",
    "-fno-exceptions",
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
