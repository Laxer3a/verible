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

// Standalone Verilog Preprocessor Library
//
// This is a standalone, buffer-based version of the Verible Verilog preprocessor.
// It is designed to work entirely in memory without filesystem dependencies.
//
// Key features:
// - Works with in-memory buffers
// - Callback-based include file resolution
// - Programmatic define management
// - Simple, minimal API

#ifndef VERIBLE_VERILOG_PREPROCESSOR_STANDALONE_VERILOG_PREPROCESSOR_H_
#define VERIBLE_VERILOG_PREPROCESSOR_STANDALONE_VERILOG_PREPROCESSOR_H_

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace verilog_preprocessor {

// Result of preprocessing operation
struct PreprocessResult {
  // The preprocessed output as a string
  std::string preprocessed_code;

  // List of error messages encountered during preprocessing
  std::vector<std::string> errors;

  // List of warning messages encountered during preprocessing
  std::vector<std::string> warnings;

  // True if preprocessing completed without errors
  bool success() const { return errors.empty(); }
};

// Callback for resolving include files
// Parameters:
//   - filename: The filename requested by the `include directive
// Returns:
//   - The content of the included file as a string
//   - Empty string if the file cannot be resolved (will generate an error)
using IncludeFileCallback =
    std::function<std::string(const std::string& filename)>;

// Define information
struct Define {
  std::string name;
  std::string value;
  std::vector<std::string> parameters;  // Empty for simple defines

  Define() = default;
  Define(std::string n, std::string v)
      : name(std::move(n)), value(std::move(v)) {}
  Define(std::string n, std::vector<std::string> params, std::string v)
      : name(std::move(n)), value(std::move(v)),
        parameters(std::move(params)) {}

  bool is_callable() const { return !parameters.empty(); }
};

// Standalone Verilog Preprocessor
//
// This class provides a simple, buffer-based interface for preprocessing
// Verilog/SystemVerilog code. It operates entirely in memory and uses
// callbacks for include file resolution.
//
// Example usage:
//   VerilogPreprocessor pp;
//   pp.AddDefine("MY_DEFINE", "42");
//   pp.SetIncludeFileCallback([](const std::string& file) {
//     // Load file from memory or custom storage
//     return file_contents;
//   });
//
//   PreprocessResult result = pp.Preprocess(source_code);
//   if (result.success()) {
//     // Use result.preprocessed_code
//   }
class VerilogPreprocessor {
 public:
  // Configuration options
  struct Config {
    // Expand macro definitions in the output
    bool expand_macros = true;

    // Process `include directives
    bool process_includes = true;

    // Filter out non-matching `ifdef/`ifndef branches
    // When true, only the matching branch is included in output
    // When false, all branches are preserved (useful for formatters)
    bool filter_branches = true;
  };

  // Construct with default configuration
  VerilogPreprocessor();

  // Construct with custom configuration
  explicit VerilogPreprocessor(const Config& config);

  // Add a simple define (without parameters)
  // Example: AddDefine("WIDTH", "32")
  //   Equivalent to: `define WIDTH 32
  void AddDefine(const std::string& name, const std::string& value = "");

  // Add a callable define (with parameters)
  // Example: AddDefineWithParams("MAX", {"a", "b"}, "((a) > (b) ? (a) : (b))")
  //   Equivalent to: `define MAX(a,b) ((a) > (b) ? (a) : (b))
  void AddDefineWithParams(const std::string& name,
                          const std::vector<std::string>& params,
                          const std::string& body);

  // Add a define structure
  void AddDefine(const Define& define);

  // Remove a define
  void RemoveDefine(const std::string& name);

  // Check if a define exists
  bool HasDefine(const std::string& name) const;

  // Get all current defines
  std::map<std::string, Define> GetDefines() const;

  // Clear all defines
  void ClearDefines();

  // Set the callback for resolving include files
  // The callback will be called whenever a `include directive is encountered
  void SetIncludeFileCallback(IncludeFileCallback callback);

  // Update configuration
  void SetConfig(const Config& config);

  // Get current configuration
  Config GetConfig() const;

  // Preprocess a buffer
  // This is the main API function. It takes Verilog source code as input
  // and returns the preprocessed result.
  PreprocessResult Preprocess(std::string_view input);

  // Preprocess with explicit base path context (for error messages)
  // The base_text parameter is used for calculating byte offsets in error messages
  PreprocessResult PreprocessWithContext(std::string_view input,
                                        std::string_view base_text);

 private:
  Config config_;
  std::map<std::string, Define> defines_;
  IncludeFileCallback include_callback_;

  // Internal implementation forward declaration
  class Impl;
};

}  // namespace verilog_preprocessor

#endif  // VERIBLE_VERILOG_PREPROCESSOR_STANDALONE_VERILOG_PREPROCESSOR_H_
