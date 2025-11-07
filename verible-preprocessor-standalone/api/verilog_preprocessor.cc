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
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "verible/common/text/text-structure.h"
#include "verible/common/text/token-info.h"
#include "verible/common/text/token-stream-view.h"
#include "verible/verilog/analysis/verilog-filelist.h"
#include "verible/verilog/parser/verilog-lexer.h"
#include "verible/verilog/preprocessor/verilog-preprocess.h"

namespace verilog_preprocessor {

// Implementation class that wraps the internal Verible preprocessor
class VerilogPreprocessor::Impl {
 public:
  explicit Impl(const Config& config) : config_(config) {}

  PreprocessResult Preprocess(std::string_view input,
                             const std::map<std::string, Define>& defines,
                             IncludeFileCallback include_callback) {
    PreprocessResult result;

    // Create text structure for the input
    auto text_structure = std::make_unique<verible::TextStructure>(input);

    // Tokenize the input
    verible::TokenSequence& token_sequence =
        text_structure->MutableData().MutableTokenStream();

    verilog::VerilogLexer lexer(input);
    for (lexer.DoNextToken(); !lexer.GetLastToken().isEOF();
         lexer.DoNextToken()) {
      token_sequence.push_back(lexer.GetLastToken());
    }

    // Create token stream view
    verible::TokenStreamView token_stream_view;
    InitTokenStreamView(token_sequence, &token_stream_view);

    // Configure the internal preprocessor
    verilog::VerilogPreprocess::Config pp_config;
    pp_config.filter_branches = config_.filter_branches;
    pp_config.include_files = config_.process_includes;
    pp_config.expand_macros = config_.expand_macros;

    // Create file opener callback if includes are enabled
    verilog::VerilogPreprocess::FileOpener file_opener;
    if (config_.process_includes && include_callback) {
      file_opener = [include_callback](
                       std::string_view filename) -> absl::StatusOr<std::string_view> {
        // Store the included content in a way that keeps it alive
        // This is done by storing it in a static/thread-local buffer
        // In practice, the callback should manage lifetime
        static thread_local std::vector<std::string> included_files;
        std::string content = include_callback(std::string(filename));
        if (content.empty()) {
          return absl::NotFoundError(
              absl::StrCat("Include file not found: ", filename));
        }
        included_files.push_back(std::move(content));
        return std::string_view(included_files.back());
      };
    }

    // Create the preprocessor
    verilog::VerilogPreprocess preprocessor(pp_config, file_opener);

    // Add defines to the preprocessor
    verilog::FileList::PreprocessingInfo preprocess_info;
    for (const auto& [name, define] : defines) {
      verilog::TextMacroDefinition macro_def;
      macro_def.name = name;
      macro_def.value = define.value;
      preprocess_info.defines.push_back(macro_def);
    }
    preprocessor.setPreprocessingInfo(preprocess_info);

    // Run preprocessing
    verilog::VerilogPreprocessData preprocess_data =
        preprocessor.ScanStream(token_stream_view);

    // Convert errors
    for (const auto& error : preprocess_data.errors) {
      std::ostringstream oss;
      oss << "Error at offset " << error.token_info.left(input) << ": "
          << error.error_message;
      result.errors.push_back(oss.str());
    }

    // Convert warnings
    for (const auto& warning : preprocess_data.warnings) {
      std::ostringstream oss;
      oss << "Warning at offset " << warning.token_info.left(input) << ": "
          << warning.error_message;
      result.warnings.push_back(oss.str());
    }

    // Build preprocessed output
    if (result.errors.empty()) {
      std::ostringstream output;
      for (const auto& token_it : preprocess_data.preprocessed_token_stream) {
        output << token_it->text();
      }
      result.preprocessed_code = output.str();
    }

    return result;
  }

 private:
  Config config_;
};

// VerilogPreprocessor implementation

VerilogPreprocessor::VerilogPreprocessor() : VerilogPreprocessor(Config()) {}

VerilogPreprocessor::VerilogPreprocessor(const Config& config)
    : config_(config) {}

void VerilogPreprocessor::AddDefine(const std::string& name,
                                   const std::string& value) {
  defines_[name] = Define(name, value);
}

void VerilogPreprocessor::AddDefineWithParams(
    const std::string& name, const std::vector<std::string>& params,
    const std::string& body) {
  defines_[name] = Define(name, params, body);
}

void VerilogPreprocessor::AddDefine(const Define& define) {
  defines_[define.name] = define;
}

void VerilogPreprocessor::RemoveDefine(const std::string& name) {
  defines_.erase(name);
}

bool VerilogPreprocessor::HasDefine(const std::string& name) const {
  return defines_.find(name) != defines_.end();
}

std::map<std::string, Define> VerilogPreprocessor::GetDefines() const {
  return defines_;
}

void VerilogPreprocessor::ClearDefines() { defines_.clear(); }

void VerilogPreprocessor::SetIncludeFileCallback(
    IncludeFileCallback callback) {
  include_callback_ = std::move(callback);
}

void VerilogPreprocessor::SetConfig(const Config& config) {
  config_ = config;
}

VerilogPreprocessor::Config VerilogPreprocessor::GetConfig() const {
  return config_;
}

PreprocessResult VerilogPreprocessor::Preprocess(std::string_view input) {
  return PreprocessWithContext(input, input);
}

PreprocessResult VerilogPreprocessor::PreprocessWithContext(
    std::string_view input, std::string_view base_text) {
  Impl impl(config_);
  return impl.Preprocess(input, defines_, include_callback_);
}

}  // namespace verilog_preprocessor
