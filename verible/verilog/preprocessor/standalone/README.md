# Standalone Verilog Preprocessor Library

A minimal, buffer-based Verilog/SystemVerilog preprocessor library designed to work entirely in memory without filesystem dependencies.

## Overview

This library extracts the core preprocessing functionality from Verible into a standalone, easy-to-use API. It's designed for applications that need to preprocess Verilog code programmatically, with full control over:

- Define management through API calls
- Include file resolution through callbacks
- In-memory buffer processing

## Features

- **Buffer-based operation**: No direct filesystem access
- **Callback-driven includes**: Custom include file resolution
- **Programmatic define management**: Add/remove defines via API
- **Configurable behavior**: Control macro expansion, branch filtering, and include processing
- **Full preprocessing support**:
  - Macro definitions and expansion
  - Conditional compilation (`ifdef`, `ifndef`, `elsif`, `else`, `endif`)
  - Include file processing
  - Callable macros with parameters
  - Nested macro expansion
  - Macro redefinition warnings

## API Overview

### Basic Usage

```cpp
#include "verible/verilog/preprocessor/standalone/verilog_preprocessor.h"

using namespace verilog_preprocessor;

VerilogPreprocessor pp;

// Add defines
pp.AddDefine("WIDTH", "32");
pp.AddDefine("DEBUG");

// Preprocess code
PreprocessResult result = pp.Preprocess(source_code);

if (result.success()) {
  // Use result.preprocessed_code
  std::cout << result.preprocessed_code;
}
```

### Key Classes

#### `VerilogPreprocessor`

The main preprocessor class with the following methods:

- `AddDefine(name, value)` - Add a simple define
- `AddDefineWithParams(name, params, body)` - Add a callable macro
- `RemoveDefine(name)` - Remove a define
- `HasDefine(name)` - Check if a define exists
- `GetDefines()` - Get all current defines
- `ClearDefines()` - Remove all defines
- `SetIncludeFileCallback(callback)` - Set include resolution callback
- `SetConfig(config)` - Configure preprocessor behavior
- `Preprocess(input)` - Preprocess a buffer

#### `PreprocessResult`

Contains the results of preprocessing:

- `preprocessed_code` - The preprocessed output
- `errors` - List of error messages
- `warnings` - List of warning messages
- `success()` - Returns true if no errors occurred

#### `Config`

Configuration options:

- `expand_macros` - Expand macro definitions (default: true)
- `process_includes` - Process `include directives (default: true)
- `filter_branches` - Filter non-matching conditional branches (default: true)

## Examples

### Example 1: Basic Defines

```cpp
VerilogPreprocessor pp;

pp.AddDefine("WIDTH", "32");
pp.AddDefine("DEPTH", "1024");
pp.AddDefine("DEBUG");

std::string code = R"(
  module my_module;
    reg [`WIDTH-1:0] data;
    `ifdef DEBUG
      initial $display("Debug mode");
    `endif
  endmodule
)";

PreprocessResult result = pp.Preprocess(code);
```

### Example 2: Callable Macros

```cpp
VerilogPreprocessor pp;

pp.AddDefineWithParams("MAX", {"a", "b"}, "((a) > (b) ? (a) : (b))");
pp.AddDefineWithParams("MIN", {"a", "b"}, "((a) < (b) ? (a) : (b))");

std::string code = R"(
  module math;
    wire [7:0] result = `MAX(x, y);
    wire [7:0] minimum = `MIN(a, b);
  endmodule
)";

PreprocessResult result = pp.Preprocess(code);
```

### Example 3: Include Files with Callback

```cpp
VerilogPreprocessor pp;

// Create an in-memory file system
std::map<std::string, std::string> files = {
  {"common.vh", "`define COMMON_WIDTH 64\n"},
  {"types.vh", "typedef logic [31:0] word_t;\n"}
};

// Set up include callback
pp.SetIncludeFileCallback([&files](const std::string& filename) {
  auto it = files.find(filename);
  if (it != files.end()) {
    return it->second;
  }
  return std::string();  // Not found
});

std::string code = R"(
  `include "common.vh"
  `include "types.vh"

  module top;
    reg [`COMMON_WIDTH-1:0] data;
    word_t value;
  endmodule
)";

PreprocessResult result = pp.Preprocess(code);
```

### Example 4: Conditional Compilation

```cpp
VerilogPreprocessor pp;

VerilogPreprocessor::Config config;
config.filter_branches = true;  // Only include matching branches
pp.SetConfig(config);

pp.AddDefine("SYNTHESIS");
pp.AddDefine("TARGET_FPGA");

std::string code = R"(
  module design;
    `ifdef SYNTHESIS
      // Synthesis-specific code
      wire optimized;
    `else
      // Simulation-specific code
      reg debug_signal;
    `endif

    `ifdef TARGET_FPGA
      // FPGA-specific code
    `elsif TARGET_ASIC
      // ASIC-specific code
    `else
      // Generic code
    `endif
  endmodule
)";

PreprocessResult result = pp.Preprocess(code);
// Output will only contain synthesis and FPGA-specific code
```

### Example 5: Error Handling

```cpp
VerilogPreprocessor pp;

std::string code = R"(
  `define INCOMPLETE
  `ifdef
  `else
  `endif
)";

PreprocessResult result = pp.Preprocess(code);

if (!result.success()) {
  for (const auto& error : result.errors) {
    std::cerr << error << "\n";
  }
}

// Check warnings
for (const auto& warning : result.warnings) {
  std::cout << "Warning: " << warning << "\n";
}
```

### Example 6: Custom Configuration

```cpp
// Create preprocessor with custom config
VerilogPreprocessor::Config config;
config.expand_macros = false;    // Don't expand macros
config.process_includes = false; // Don't process includes
config.filter_branches = false;  // Keep all branches

VerilogPreprocessor pp(config);

// This is useful for formatters that need to preserve
// the original structure while still parsing defines
```

## Building

### With Bazel

```bash
# Build the library
bazel build //verible/verilog/preprocessor/standalone:verilog-preprocessor-standalone

# Build and run the example
bazel run //verible/verilog/preprocessor/standalone:verilog-preprocessor-example

# Run tests
bazel test //verible/verilog/preprocessor/standalone:verilog-preprocessor-standalone_test
```

### Including in Your Project

Add to your BUILD file:

```python
cc_binary(
    name = "my_app",
    srcs = ["my_app.cc"],
    deps = [
        "//verible/verilog/preprocessor/standalone:verilog-preprocessor-standalone",
    ],
)
```

## Design Philosophy

This standalone library is designed around the following principles:

1. **Memory-based operation**: All input and output is through strings and buffers, no direct file I/O
2. **Callback-driven**: Include files are resolved through user-provided callbacks, allowing integration with any storage system
3. **Explicit control**: Defines are managed programmatically through API calls, not command-line flags
4. **Minimal dependencies**: Only depends on core Verible components needed for preprocessing
5. **Simple API**: Easy to integrate into existing applications

## Limitations and Design Choices

This is a **pseudo-preprocessor** rather than a fully standards-compliant preprocessor:

- Does not evaluate preprocessor expressions (expressions in `ifdef are not computed)
- Best-effort handling of directives
- Some advanced features like token concatenation (``) and stringification are not yet implemented

These limitations are by design, as the library is intended for use in tools like:
- Source code formatters
- Linters and analyzers
- Custom build systems
- Language servers

For these applications, perfect preprocessing is not required or even desired, as they need to handle code in various states of completeness.

## Thread Safety

The `VerilogPreprocessor` class is **not** thread-safe. Create separate instances for different threads, or protect access with appropriate synchronization.

## Performance Considerations

- The preprocessor performs lexical analysis on all input
- Macro expansion can be recursive and may increase processing time
- Include files are processed recursively
- For large codebases, consider caching preprocessed results

## Comparison with Full Preprocessor

This standalone library differs from a full Verilog preprocessor:

| Feature | Standalone Library | Full Preprocessor |
|---------|-------------------|-------------------|
| Filesystem access | No (callback-based) | Yes |
| Expression evaluation | No | Yes |
| Standard compliance | Best-effort | Full |
| Token concatenation | No | Yes |
| Stringification | No | Yes |
| Include path search | No (callback handles) | Yes |
| Intended use | Tools/analyzers | Compilation |

## Contributing

This library is part of the Verible project. For contributions, please see the main Verible repository.

## License

Apache License 2.0 - See LICENSE file for details

## Related Documentation

- [Verible Project](https://github.com/chipsalliance/verible)
- [Main Preprocessor README](../README.md)
- [Verilog Preprocessor Specification](https://ieeexplore.ieee.org/document/8299595)
