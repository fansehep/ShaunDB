workspace(name = "fver")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")



# generate compile_commands.json
http_archive(
    name = "hedron_compile_commands",

    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here. 
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    urls = [
        "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/af9af15f7bc16fc3e407e2231abfcb62907d258f.tar.gz",
    ],
    strip_prefix = "bazel-compile-commands-extractor-af9af15f7bc16fc3e407e2231abfcb62907d258f",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..." 
)
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
hedron_compile_commands_setup()
