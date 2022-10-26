workspace(name="fver")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")



http_archive(
    name="com_google_googletest",
    sha256="a1d3123179024258f9c399d45da3e0b09c4aaf8d2c041466ce5b4793a8929f23",
    strip_prefix="googletest-86add13493e5c881d7e4ba77fb91c1f57752b3a4",
    urls=["https://github.com/google/googletest/archive/86add13493e5c881d7e4ba77fb91c1f57752b3a4.zip"],
)

http_archive(
    name="com_google_benchmark",  # 2021-09-20T09:19:51Z
    sha256="62e2f2e6d8a744d67e4bbc212fcfd06647080de4253c97ad5c6749e09faf2cb0",
    strip_prefix="benchmark-0baacde3618ca617da95375e0af13ce1baadea47",
    urls=["https://github.com/google/benchmark/archive/0baacde3618ca617da95375e0af13ce1baadea47.zip"],
)
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "fmt",
    branch = "master",
    remote = "https://github.com/fmtlib/fmt",
    patch_cmds = [
        "mv support/bazel/.bazelrc .bazelrc",
        "mv support/bazel/.bazelversion .bazelversion",
        "mv support/bazel/BUILD.bazel BUILD.bazel",
        "mv support/bazel/WORKSPACE.bazel WORKSPACE.bazel",
    ],
    # Windows-related patch commands are only needed in the case MSYS2 is not installed.
    # More details about the installation process of MSYS2 on Windows systems can be found here:
    # https://docs.bazel.build/versions/main/install-windows.html#installing-compilers-and-language-runtimes
    # Even if MSYS2 is installed the Windows related patch commands can still be used.
    patch_cmds_win = [
        "Move-Item -Path support/bazel/.bazelrc -Destination .bazelrc",
        "Move-Item -Path support/bazel/.bazelversion -Destination .bazelversion",
        "Move-Item -Path support/bazel/BUILD.bazel -Destination BUILD.bazel",
        "Move-Item -Path support/bazel/WORKSPACE.bazel -Destination WORKSPACE.bazel",
    ],
)

# generate compile_commands.json
http_archive(
    name="hedron_compile_commands",

    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here.
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    urls=[
        "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/af9af15f7bc16fc3e407e2231abfcb62907d258f.tar.gz",
    ],
    strip_prefix="bazel-compile-commands-extractor-af9af15f7bc16fc3e407e2231abfcb62907d258f",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..."
)
load("@hedron_compile_commands//:workspace_setup.bzl",
     "hedron_compile_commands_setup")
hedron_compile_commands_setup()
