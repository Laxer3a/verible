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

// Example usage of the standalone Verilog preprocessor library

#include <iostream>
#include <map>
#include <string>

#include "verilog_preprocessor.h"

using namespace verilog_preprocessor;

// Example 1: Basic define and macro expansion
void BasicDefineExample() {
  std::cout << "\n=== Example 1: Basic Define ===\n";

  VerilogPreprocessor pp;

  // Add some defines
  pp.AddDefine("WIDTH", "32");
  pp.AddDefine("DEPTH", "1024");
  pp.AddDefine("DEBUG");  // Define without value

  std::string code = R"(
    module my_module;
      reg [`WIDTH-1:0] data;
      reg [`DEPTH-1:0] memory;
      `ifdef DEBUG
        initial $display("Debug mode enabled");
      `endif
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "Preprocessed code:\n" << result.preprocessed_code << "\n";
  } else {
    std::cout << "Errors:\n";
    for (const auto& error : result.errors) {
      std::cout << "  " << error << "\n";
    }
  }
}

// Example 2: Callable macros with parameters
void CallableMacroExample() {
  std::cout << "\n=== Example 2: Callable Macros ===\n";

  VerilogPreprocessor pp;

  // Add a macro with parameters
  pp.AddDefineWithParams("MAX", {"a", "b"}, "((a) > (b) ? (a) : (b))");
  pp.AddDefineWithParams("MIN", {"a", "b"}, "((a) < (b) ? (a) : (b))");
  pp.AddDefineWithParams("CLAMP", {"val", "min", "max"},
                        "`MAX(`MIN(val, max), min)");

  std::string code = R"(
    module math_ops;
      wire [7:0] result;
      assign result = `MAX(10, 20);
      wire [7:0] clamped = `CLAMP(value, 0, 255);
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "Preprocessed code:\n" << result.preprocessed_code << "\n";
  } else {
    std::cout << "Errors:\n";
    for (const auto& error : result.errors) {
      std::cout << "  " << error << "\n";
    }
  }
}

// Example 3: Include file handling with callback
void IncludeFileExample() {
  std::cout << "\n=== Example 3: Include Files ===\n";

  VerilogPreprocessor pp;

  // Create a simple in-memory file system
  std::map<std::string, std::string> file_system = {
      {"common.vh", R"(
        `define COMMON_WIDTH 64
        `define COMMON_DEPTH 2048
      )"},
      {"debug.vh", R"(
        `define DEBUG_LEVEL 2
        `ifdef ENABLE_TRACE
          `define TRACE_ON
        `endif
      )"}};

  // Set up the include callback
  pp.SetIncludeFileCallback([&file_system](const std::string& filename) {
    auto it = file_system.find(filename);
    if (it != file_system.end()) {
      std::cout << "  Loading include file: " << filename << "\n";
      return it->second;
    }
    std::cerr << "  File not found: " << filename << "\n";
    return std::string();  // Return empty string if not found
  });

  pp.AddDefine("ENABLE_TRACE");

  std::string code = R"(
    `include "common.vh"
    `include "debug.vh"

    module top;
      reg [`COMMON_WIDTH-1:0] data;
      `ifdef TRACE_ON
        initial $display("Tracing enabled at level %0d", `DEBUG_LEVEL);
      `endif
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "Preprocessed code:\n" << result.preprocessed_code << "\n";
  } else {
    std::cout << "Errors:\n";
    for (const auto& error : result.errors) {
      std::cout << "  " << error << "\n";
    }
  }

  // Show warnings if any
  if (!result.warnings.empty()) {
    std::cout << "Warnings:\n";
    for (const auto& warning : result.warnings) {
      std::cout << "  " << warning << "\n";
    }
  }
}

// Example 4: Conditional compilation
void ConditionalCompilationExample() {
  std::cout << "\n=== Example 4: Conditional Compilation ===\n";

  VerilogPreprocessor pp;

  // Configure to filter branches
  VerilogPreprocessor::Config config;
  config.filter_branches = true;
  config.expand_macros = true;
  pp.SetConfig(config);

  pp.AddDefine("SYNTHESIS");
  pp.AddDefine("TARGET_FPGA", "1");

  std::string code = R"(
    module design;
      `ifdef SYNTHESIS
        // Synthesis path
        wire [31:0] optimized_logic;
      `else
        // Simulation path
        reg [31:0] debug_signals;
        initial $display("Debug build");
      `endif

      `ifdef TARGET_FPGA
        `ifdef XILINX
          // Xilinx-specific code
        `elsif ALTERA
          // Altera-specific code
        `else
          // Generic FPGA code
          wire generic_fpga;
        `endif
      `endif
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "Preprocessed code (filtered):\n"
              << result.preprocessed_code << "\n";
  }
}

// Example 5: Advanced configuration
void AdvancedConfigExample() {
  std::cout << "\n=== Example 5: Advanced Configuration ===\n";

  // Configure preprocessor with custom settings
  VerilogPreprocessor::Config config;
  config.expand_macros = false;      // Don't expand macros (keep them as-is)
  config.process_includes = false;   // Don't process includes
  config.filter_branches = false;    // Keep all conditional branches

  VerilogPreprocessor pp(config);

  pp.AddDefine("MY_MACRO", "some_value");

  std::string code = R"(
    `define LOCAL_DEFINE 42
    module test;
      wire x = `MY_MACRO;
      wire y = `LOCAL_DEFINE;
      `ifdef SOME_FLAG
        // Branch 1
      `else
        // Branch 2
      `endif
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "Preprocessed code (no expansion/filtering):\n"
              << result.preprocessed_code << "\n";
  }

  // Now show the difference with expansion enabled
  config.expand_macros = true;
  config.filter_branches = true;
  pp.SetConfig(config);

  result = pp.Preprocess(code);

  if (result.success()) {
    std::cout << "\nWith expansion and filtering:\n"
              << result.preprocessed_code << "\n";
  }
}

// Example 6: Error handling
void ErrorHandlingExample() {
  std::cout << "\n=== Example 6: Error Handling ===\n";

  VerilogPreprocessor pp;

  std::string code = R"(
    module test;
      `ifdef
      // Error: missing identifier after ifdef

      `define
      // Error: incomplete define

      wire x = `UNDEFINED_MACRO;
      // Error: undefined macro

      `else
      // Error: else without ifdef

      `endif
      // Error: endif without ifdef
    endmodule
  )";

  PreprocessResult result = pp.Preprocess(code);

  std::cout << "Errors encountered:\n";
  for (const auto& error : result.errors) {
    std::cout << "  " << error << "\n";
  }

  std::cout << "\nSuccess: " << (result.success() ? "Yes" : "No") << "\n";
}

int main() {
  std::cout << "========================================\n";
  std::cout << "Standalone Verilog Preprocessor Examples\n";
  std::cout << "========================================\n";

  BasicDefineExample();
  CallableMacroExample();
  IncludeFileExample();
  ConditionalCompilationExample();
  AdvancedConfigExample();
  ErrorHandlingExample();

  std::cout << "\n=== All examples completed ===\n";
  return 0;
}
