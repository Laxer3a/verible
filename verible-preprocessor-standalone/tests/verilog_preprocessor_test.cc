// Copyright 2017-2020 The Verible Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "verilog_preprocessor.h"

#include <map>
#include <string>

#include "gtest/gtest.h"

namespace verilog_preprocessor {
namespace {

TEST(VerilogPreprocessorTest, BasicDefine) {
  VerilogPreprocessor pp;
  pp.AddDefine("MY_DEFINE", "42");

  std::string code = "`define LOCAL_MACRO 123\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
  EXPECT_FALSE(result.preprocessed_code.empty());
}

TEST(VerilogPreprocessorTest, DefineWithoutValue) {
  VerilogPreprocessor pp;
  pp.AddDefine("FLAG");

  std::string code = "`ifdef FLAG\nwire x;\n`endif\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
}

TEST(VerilogPreprocessorTest, MultipleDefines) {
  VerilogPreprocessor pp;
  pp.AddDefine("WIDTH", "32");
  pp.AddDefine("DEPTH", "1024");
  pp.AddDefine("DEBUG");

  EXPECT_TRUE(pp.HasDefine("WIDTH"));
  EXPECT_TRUE(pp.HasDefine("DEPTH"));
  EXPECT_TRUE(pp.HasDefine("DEBUG"));
  EXPECT_FALSE(pp.HasDefine("UNDEFINED"));

  auto defines = pp.GetDefines();
  EXPECT_EQ(defines.size(), 3);
  EXPECT_EQ(defines["WIDTH"].value, "32");
  EXPECT_EQ(defines["DEPTH"].value, "1024");
}

TEST(VerilogPreprocessorTest, RemoveDefine) {
  VerilogPreprocessor pp;
  pp.AddDefine("TEMP", "value");
  EXPECT_TRUE(pp.HasDefine("TEMP"));

  pp.RemoveDefine("TEMP");
  EXPECT_FALSE(pp.HasDefine("TEMP"));
}

TEST(VerilogPreprocessorTest, ClearDefines) {
  VerilogPreprocessor pp;
  pp.AddDefine("A", "1");
  pp.AddDefine("B", "2");
  pp.AddDefine("C", "3");

  EXPECT_EQ(pp.GetDefines().size(), 3);

  pp.ClearDefines();
  EXPECT_EQ(pp.GetDefines().size(), 0);
}

TEST(VerilogPreprocessorTest, CallableMacro) {
  VerilogPreprocessor pp;
  pp.AddDefineWithParams("MAX", {"a", "b"}, "((a) > (b) ? (a) : (b))");

  std::string code = "wire x = `MAX(10, 20);\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
}

TEST(VerilogPreprocessorTest, ConditionalCompilation) {
  VerilogPreprocessor pp;

  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  pp.SetConfig(config);

  pp.AddDefine("FEATURE_A");

  std::string code = R"(
    `ifdef FEATURE_A
      wire feature_a_signal;
    `else
      wire feature_b_signal;
    `endif
  )";

  PreprocessResult result = pp.Preprocess(code);
  EXPECT_TRUE(result.success());
  // Should contain feature_a_signal but not feature_b_signal
  EXPECT_NE(result.preprocessed_code.find("feature_a_signal"),
            std::string::npos);
}

TEST(VerilogPreprocessorTest, IfdefNotDefined) {
  VerilogPreprocessor pp;

  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  pp.SetConfig(config);

  // Don't define FEATURE_A

  std::string code = R"(
    `ifdef FEATURE_A
      wire feature_a_signal;
    `else
      wire feature_b_signal;
    `endif
  )";

  PreprocessResult result = pp.Preprocess(code);
  EXPECT_TRUE(result.success());
  // Should contain feature_b_signal but not feature_a_signal
  EXPECT_NE(result.preprocessed_code.find("feature_b_signal"),
            std::string::npos);
}

TEST(VerilogPreprocessorTest, IncludeFileCallback) {
  VerilogPreprocessor pp;

  std::map<std::string, std::string> file_system = {
      {"header.vh", "`define HEADER_LOADED 1\n"}};

  pp.SetIncludeFileCallback([&file_system](const std::string& filename) {
    auto it = file_system.find(filename);
    return it != file_system.end() ? it->second : std::string();
  });

  std::string code = "`include \"header.vh\"\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
  EXPECT_NE(result.preprocessed_code.find("HEADER_LOADED"), std::string::npos);
}

TEST(VerilogPreprocessorTest, IncludeFileNotFound) {
  VerilogPreprocessor pp;

  pp.SetIncludeFileCallback([](const std::string& filename) {
    return std::string();  // File not found
  });

  std::string code = "`include \"nonexistent.vh\"\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_FALSE(result.success());
  EXPECT_FALSE(result.errors.empty());
}

TEST(VerilogPreprocessorTest, NestedIncludes) {
  VerilogPreprocessor pp;

  std::map<std::string, std::string> file_system = {
      {"a.vh", "`include \"b.vh\"\n`define A_LOADED\n"},
      {"b.vh", "`define B_LOADED\n"}};

  pp.SetIncludeFileCallback([&file_system](const std::string& filename) {
    auto it = file_system.find(filename);
    return it != file_system.end() ? it->second : std::string();
  });

  std::string code = "`include \"a.vh\"\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
  EXPECT_NE(result.preprocessed_code.find("B_LOADED"), std::string::npos);
  EXPECT_NE(result.preprocessed_code.find("A_LOADED"), std::string::npos);
}

TEST(VerilogPreprocessorTest, ConfigExpandMacros) {
  VerilogPreprocessor pp;

  // Test with expansion disabled
  VerilogPreprocessor::Config config;
  config.expand_macros = false;
  pp.SetConfig(config);

  pp.AddDefine("MY_MACRO", "value");

  std::string code = "`define LOCAL 123\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());

  // Test with expansion enabled
  config.expand_macros = true;
  pp.SetConfig(config);

  result = pp.Preprocess(code);
  EXPECT_TRUE(result.success());
}

TEST(VerilogPreprocessorTest, ConfigFilterBranches) {
  VerilogPreprocessor pp;
  pp.AddDefine("FEATURE");

  // Test with filtering enabled
  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  pp.SetConfig(config);

  std::string code = "`ifdef FEATURE\nwire a;\n`else\nwire b;\n`endif\n";
  PreprocessResult result1 = pp.Preprocess(code);

  // Test with filtering disabled
  config.filter_branches = false;
  pp.SetConfig(config);

  PreprocessResult result2 = pp.Preprocess(code);

  EXPECT_TRUE(result1.success());
  EXPECT_TRUE(result2.success());
  // With filtering, only 'wire a' should be present
  // Without filtering, both branches should be present
}

TEST(VerilogPreprocessorTest, DefineStruct) {
  Define def1("SIMPLE", "value");
  EXPECT_EQ(def1.name, "SIMPLE");
  EXPECT_EQ(def1.value, "value");
  EXPECT_FALSE(def1.is_callable());

  Define def2("CALLABLE", {"a", "b"}, "a + b");
  EXPECT_EQ(def2.name, "CALLABLE");
  EXPECT_EQ(def2.value, "a + b");
  EXPECT_TRUE(def2.is_callable());
  EXPECT_EQ(def2.parameters.size(), 2);
}

TEST(VerilogPreprocessorTest, EmptyInput) {
  VerilogPreprocessor pp;

  PreprocessResult result = pp.Preprocess("");
  EXPECT_TRUE(result.success());
  EXPECT_TRUE(result.preprocessed_code.empty());
}

TEST(VerilogPreprocessorTest, PreprocessResultSuccess) {
  PreprocessResult result1;
  EXPECT_TRUE(result1.success());

  PreprocessResult result2;
  result2.errors.push_back("error");
  EXPECT_FALSE(result2.success());
}

TEST(VerilogPreprocessorTest, ComplexMacroExpansion) {
  VerilogPreprocessor pp;

  pp.AddDefineWithParams("ADD", {"a", "b"}, "a + b");
  pp.AddDefineWithParams("MUL", {"x", "y"}, "x * y");
  pp.AddDefineWithParams("EXPR", {"p", "q"},
                        "`ADD(`MUL(p, 2), `MUL(q, 3))");

  std::string code = "wire result = `EXPR(a, b);\n";
  PreprocessResult result = pp.Preprocess(code);

  EXPECT_TRUE(result.success());
}

TEST(VerilogPreprocessorTest, Ifndef) {
  VerilogPreprocessor pp;

  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  pp.SetConfig(config);

  // Test ifndef when symbol is not defined
  std::string code1 = "`ifndef UNDEFINED\nwire a;\n`endif\n";
  PreprocessResult result1 = pp.Preprocess(code1);
  EXPECT_TRUE(result1.success());
  EXPECT_NE(result1.preprocessed_code.find("wire a"), std::string::npos);

  // Test ifndef when symbol is defined
  pp.AddDefine("DEFINED");
  std::string code2 = "`ifndef DEFINED\nwire b;\n`endif\n";
  PreprocessResult result2 = pp.Preprocess(code2);
  EXPECT_TRUE(result2.success());
  // wire b should not be in output
}

TEST(VerilogPreprocessorTest, ElsifChain) {
  VerilogPreprocessor pp;

  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  pp.SetConfig(config);

  pp.AddDefine("OPTION_B");

  std::string code = R"(
    `ifdef OPTION_A
      wire a;
    `elsif OPTION_B
      wire b;
    `elsif OPTION_C
      wire c;
    `else
      wire d;
    `endif
  )";

  PreprocessResult result = pp.Preprocess(code);
  EXPECT_TRUE(result.success());
  EXPECT_NE(result.preprocessed_code.find("wire b"), std::string::npos);
}

}  // namespace
}  // namespace verilog_preprocessor
