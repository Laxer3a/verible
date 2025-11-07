# Standalone Verilog Preprocessor Library
#
# This is a minimal, buffer-based preprocessor library that can be used
# independently of the main Verible infrastructure.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(
    default_applicable_licenses = ["//:license"],
    default_visibility = ["//visibility:public"],
    features = ["layering_check"],
)

cc_library(
    name = "verilog-preprocessor-standalone",
    srcs = ["verilog_preprocessor.cc"],
    hdrs = ["verilog_preprocessor.h"],
    deps = [
        "//verible/common/text:text-structure",
        "//verible/common/text:token-info",
        "//verible/common/text:token-stream-view",
        "//verible/verilog/analysis:verilog-filelist",
        "//verible/verilog/parser:verilog-lexer",
        "//verible/verilog/preprocessor:verilog-preprocess",
        "@abseil-cpp//absl/status",
        "@abseil-cpp//absl/status:statusor",
        "@abseil-cpp//absl/strings",
    ],
)

cc_binary(
    name = "verilog-preprocessor-example",
    srcs = ["example.cc"],
    deps = [
        ":verilog-preprocessor-standalone",
    ],
)

cc_test(
    name = "verilog-preprocessor-standalone_test",
    srcs = ["verilog_preprocessor_test.cc"],
    deps = [
        ":verilog-preprocessor-standalone",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
