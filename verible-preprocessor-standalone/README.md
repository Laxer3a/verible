# Standalone Verilog Preprocessor Library

A minimal, buffer-based Verilog/SystemVerilog preprocessor library extracted from the Verible project.

## 🪟 Windows Users

**Quick Start:** See **[QUICKSTART_WINDOWS.md](QUICKSTART_WINDOWS.md)** for a 10-minute setup guide!

**Detailed Guide:** See **[WINDOWS_BUILD.md](WINDOWS_BUILD.md)** for complete Visual Studio instructions.

## Features

- **Buffer-based operation**: No direct filesystem access
- **Callback-driven includes**: Custom include file resolution
- **Programmatic define management**: Add/remove defines via API
- **Full preprocessing support**: Macros, conditionals, includes, nested expansion
- **Cross-platform**: Linux, macOS, Windows (Visual Studio)

## Building

### Prerequisites

- CMake 3.15 or later
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Flex 2.5 or later
- **Abseil C++ library >= 20230125** ⚠️ **IMPORTANT - See below**
- (Optional) Google Test for unit tests

### ⚠️ Important: Abseil Version Requirement

**Minimum Required: Abseil LTS 20230125 or later**

- ❌ Ubuntu 22.04 (Abseil 20220623) - **TOO OLD** - See [DEPENDENCIES.md](DEPENDENCIES.md) for upgrade instructions
- ✅ Ubuntu 24.04 (Abseil 20240116+) - **Works out of the box**
- ✅ Windows vcpkg - **Works** (always provides latest)
- ✅ macOS Homebrew - **Works** (provides latest)

**If you're on Ubuntu 22.04 or Debian 11**, you'll need to build Abseil from source. See **[DEPENDENCIES.md](DEPENDENCIES.md)** for detailed instructions.

### Ubuntu 24.04+ / Debian 12+ (Recommended)

```bash
sudo apt-get install cmake g++ flex libabsl-dev libgtest-dev
```

### Ubuntu 22.04 / Debian 11 (Requires Abseil Upgrade)

See **[DEPENDENCIES.md](DEPENDENCIES.md)** for instructions on building Abseil from source.

### Windows

**Quick Start:**
```powershell
# Using the build script
.\build.bat

# Or manually
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

**📘 For detailed Windows instructions, see [WINDOWS_BUILD.md](WINDOWS_BUILD.md)**

This includes:
- Visual Studio setup
- Installing dependencies with vcpkg
- Building with Visual Studio IDE
- Building from command line
- Troubleshooting common issues

### macOS

```bash
brew install cmake flex abseil googletest
```

### Build Instructions (Linux/macOS)

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Build Options

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_EXAMPLES=ON
```

### Install

```bash
sudo make install
```

This installs:
- Library: `/usr/local/lib/libverilog_preprocessor.a`
- Header: `/usr/local/include/verilog_preprocessor.h`
- Example: `/usr/local/bin/preprocessor_example`
- Tests: `/usr/local/bin/preprocessor_tests`

## Quick Start

### Basic Usage

```cpp
#include "verilog_preprocessor.h"

using namespace verilog_preprocessor;

VerilogPreprocessor pp;

// Add defines
pp.AddDefine("WIDTH", "32");
pp.AddDefine("DEBUG");

// Preprocess code
std::string code = R"(
    module test;
        reg [`WIDTH-1:0] data;
        `ifdef DEBUG
            initial $display("Debug mode");
        `endif
    endmodule
)";

PreprocessResult result = pp.Preprocess(code);

if (result.success()) {
    std::cout << result.preprocessed_code;
} else {
    for (const auto& error : result.errors) {
        std::cerr << error << "\n";
    }
}
```

### With Include Callback

```cpp
VerilogPreprocessor pp;

// Set up in-memory file system
std::map<std::string, std::string> files = {
    {"common.vh", "`define BUS_WIDTH 64\n"}
};

pp.SetIncludeFileCallback([&files](const std::string& filename) {
    auto it = files.find(filename);
    return (it != files.end()) ? it->second : std::string();
});

std::string code = "`include \"common.vh\"\n";
PreprocessResult result = pp.Preprocess(code);
```

### Callable Macros

```cpp
VerilogPreprocessor pp;

pp.AddDefineWithParams("MAX", {"a", "b"}, "((a) > (b) ? (a) : (b))");

std::string code = "wire result = `MAX(x, y);";
PreprocessResult result = pp.Preprocess(code);
```

## API Reference

### VerilogPreprocessor Class

#### Methods

- `AddDefine(name, value)` - Add a simple define
- `AddDefineWithParams(name, params, body)` - Add a callable macro
- `RemoveDefine(name)` - Remove a define
- `HasDefine(name)` - Check if a define exists
- `GetDefines()` - Get all current defines
- `ClearDefines()` - Remove all defines
- `SetIncludeFileCallback(callback)` - Set include resolution callback
- `SetConfig(config)` - Configure preprocessor behavior
- `Preprocess(input)` - Preprocess a buffer

### PreprocessResult Structure

- `preprocessed_code` - The preprocessed output
- `errors` - List of error messages
- `warnings` - List of warning messages
- `success()` - Returns true if no errors

### Config Structure

- `expand_macros` - Expand macro definitions (default: true)
- `process_includes` - Process `include directives (default: true)
- `filter_branches` - Filter non-matching conditional branches (default: true)

## Running Examples

```bash
./build/preprocessor_example
```

## Running Tests

```bash
./build/preprocessor_tests
# or
ctest
```

## Project Structure

```
verible-preprocessor-standalone/
├── api/                        # Simple C++ API wrapper
│   ├── verilog_preprocessor.h
│   └── verilog_preprocessor.cc
├── verible/                    # Vendored Verible code
│   ├── common/
│   │   ├── text/              # Token and text utilities
│   │   ├── lexer/             # Lexer infrastructure
│   │   ├── util/              # Utility functions
│   │   └── strings/           # String utilities
│   └── verilog/
│       ├── preprocessor/      # Core preprocessor
│       ├── parser/            # Verilog lexer
│       └── analysis/          # Analysis utilities
├── examples/
│   └── example.cc
├── tests/
│   └── verilog_preprocessor_test.cc
├── CMakeLists.txt
└── README.md
```

## Dependencies

This standalone library includes:
- Core Verible preprocessor and lexer
- Essential text and token utilities
- All necessary support infrastructure

The only external dependencies are:
- Abseil C++ library (for status codes and utilities)
- Flex (build-time only, for generating lexer)
- Google Test (optional, for tests)

## Differences from Full Verible

This is a **pseudo-preprocessor** optimized for tools:
- Does not evaluate preprocessor expressions
- Best-effort handling of directives
- Some advanced features (token concatenation, stringification) not implemented

These limitations are intentional, as the library is designed for:
- Source code formatters
- Linters and analyzers
- Custom build systems
- Language servers

## License

Apache License 2.0 - See LICENSE file

## Credits

Extracted from the [Verible project](https://github.com/chipsalliance/verible) by the Verible Authors.
