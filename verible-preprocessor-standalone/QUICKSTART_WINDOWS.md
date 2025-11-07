# Windows Quick Start Guide

Get the Verilog preprocessor running on Windows in under 10 minutes!

## Prerequisites

You need:
- ✅ Visual Studio 2017 or later (Community Edition is free)
- ✅ Git for Windows (optional but recommended)

## Step-by-Step Installation

### 1. Install vcpkg (Package Manager)

Open PowerShell and run:

```powershell
# Navigate to C:\ drive
cd C:\

# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git

# Bootstrap vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat

# Integrate with Visual Studio
.\vcpkg integrate install
```

⏱️ Takes about 2-3 minutes

### 2. Install Dependencies

```powershell
# Install Abseil (required)
.\vcpkg install abseil:x64-windows

# Install Flex for Windows (required)
.\vcpkg install winflexbison:x64-windows

# Install Google Test (optional, for tests)
.\vcpkg install gtest:x64-windows
```

⏱️ Takes about 10-15 minutes (downloads and compiles libraries)

☕ Time for coffee!

### 3. Get the Preprocessor Code

```powershell
# Navigate to your projects folder
cd C:\projects

# If you have git:
git clone <your-repo-url>
cd verible-preprocessor-standalone

# Or extract the ZIP file to C:\projects\verible-preprocessor-standalone
```

### 4. Build

**Option A: Using the build script (easiest)**

Open "x64 Native Tools Command Prompt for VS 2022" (or your VS version) from Start Menu, then:

```powershell
cd C:\projects\verible-preprocessor-standalone
.\build.bat
```

That's it! The script will:
- ✅ Detect vcpkg automatically
- ✅ Configure CMake
- ✅ Build the library
- ✅ Offer to run examples and tests

**Option B: Manual build**

```powershell
cd C:\projects\verible-preprocessor-standalone
mkdir build
cd build

# Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# Build
cmake --build . --config Release

# Test
Release\preprocessor_example.exe
```

## Using Visual Studio IDE

### Method 1: Open Folder (Recommended)

1. Launch Visual Studio
2. **File** → **Open** → **Folder**
3. Select the `verible-preprocessor-standalone` folder
4. Wait for CMake to configure (watch the Output window)
5. **Build** → **Build All** (Ctrl+Shift+B)
6. In Solution Explorer, right-click `preprocessor_example.exe` → **Execute**

### Method 2: Generate Solution File

```powershell
cd verible-preprocessor-standalone
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Then open `VerilogPreprocessorStandalone.sln` in Visual Studio.

## Verify Installation

After building, you should see:

```
verible-preprocessor-standalone\build\Release\
├── verilog_preprocessor.lib     ← The library
├── preprocessor_example.exe     ← Example program
└── preprocessor_tests.exe       ← Test suite
```

Run the example:
```powershell
cd build\Release
.\preprocessor_example.exe
```

You should see output demonstrating various preprocessor features!

## Quick Test: Your First Program

Create `test.cpp`:

```cpp
#include "verilog_preprocessor.h"
#include <iostream>

int main() {
    using namespace verilog_preprocessor;

    VerilogPreprocessor pp;
    pp.AddDefine("WIDTH", "32");

    std::string code = "wire [`WIDTH-1:0] data;";
    PreprocessResult result = pp.Preprocess(code);

    if (result.success()) {
        std::cout << "Success: " << result.preprocessed_code << "\n";
    }

    return 0;
}
```

Compile and run:
```powershell
cl /EHsc /std:c++17 test.cpp ^
   /I C:\projects\verible-preprocessor-standalone\api ^
   /link C:\projects\verible-preprocessor-standalone\build\Release\verilog_preprocessor.lib
```

## Troubleshooting

### "Flex not found"

Make sure you installed winflexbison:
```powershell
cd C:\vcpkg
.\vcpkg install winflexbison:x64-windows
```

### "Cannot find absl::..."

Make sure you installed abseil:
```powershell
cd C:\vcpkg
.\vcpkg install abseil:x64-windows
```

And use the vcpkg toolchain:
```powershell
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

### "Visual Studio compiler not found"

You need to run from the Visual Studio command prompt:
- Search for "x64 Native Tools Command Prompt" in Start Menu
- Or run: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`

### Build is slow

Use Ninja for faster builds:
```powershell
vcpkg install ninja:x64-windows
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
```

### Still having issues?

See the detailed guide: **[WINDOWS_BUILD.md](WINDOWS_BUILD.md)**

## Common Commands Cheat Sheet

```powershell
# Clean build
.\build.bat clean

# Debug build
.\build.bat Debug

# Build without tests
cmake .. -DBUILD_TESTS=OFF

# Install to custom location
cmake --install . --prefix C:\my-location

# Run specific test
Release\preprocessor_tests.exe --gtest_filter=*DefineTest*
```

## Next Steps

1. ✅ Read the [README.md](README.md) for API documentation
2. ✅ Check [examples/example.cc](examples/example.cc) for usage examples
3. ✅ Review [WINDOWS_BUILD.md](WINDOWS_BUILD.md) for advanced topics
4. ✅ Integrate into your project (see README.md)

## Complete Example: Fresh Windows Install

```powershell
# 1. Install vcpkg (one-time)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. Install dependencies (one-time, ~15 minutes)
.\vcpkg install abseil:x64-windows gtest:x64-windows winflexbison:x64-windows

# 3. Get the code
cd C:\projects
# Extract or git clone here

# 4. Build (from VS command prompt)
cd verible-preprocessor-standalone
.\build.bat

# 5. Done! Run example
build\Release\preprocessor_example.exe
```

🎉 **You're ready to use the Verilog preprocessor on Windows!**

## Support

- 📖 Full Windows guide: [WINDOWS_BUILD.md](WINDOWS_BUILD.md)
- 📖 General documentation: [README.md](README.md)
- 📖 API reference: See README.md
- 🔧 Troubleshooting: See WINDOWS_BUILD.md
