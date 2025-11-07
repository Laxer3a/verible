# Building on Windows with Visual Studio

This guide covers building the standalone Verilog preprocessor on Windows using Visual Studio.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installing Dependencies](#installing-dependencies)
3. [Building with Visual Studio IDE](#building-with-visual-studio-ide)
4. [Building from Command Line](#building-from-command-line)
5. [Using vcpkg for Dependencies](#using-vcpkg-for-dependencies)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Software

1. **Visual Studio 2017 or later**
   - Download from: https://visualstudio.microsoft.com/
   - Required components:
     - Desktop development with C++
     - C++ CMake tools for Windows
     - C++ build tools
   - Free Community Edition is sufficient

2. **CMake 3.15 or later**
   - Download from: https://cmake.org/download/
   - During installation, select "Add CMake to system PATH"
   - Or install via Visual Studio installer

3. **Git for Windows** (optional but recommended)
   - Download from: https://git-scm.com/download/win

### Required Libraries

- **Abseil C++** - Google's C++ library
- **Flex** - Lexical analyzer generator
- **Google Test** (optional) - For unit tests

## Installing Dependencies

### Option 1: Using vcpkg (Recommended)

vcpkg is Microsoft's C++ package manager and the easiest way to install dependencies.

#### Step 1: Install vcpkg

```powershell
# Open PowerShell and run:
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

#### Step 2: Install Abseil and GTest

```powershell
# Install dependencies (this may take 10-20 minutes)
.\vcpkg install abseil:x64-windows
.\vcpkg install gtest:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### Step 3: Install Win flex-bison

Flex for Windows is available as "win_flex_bison":

```powershell
.\vcpkg install winflexbison:x64-windows
```

Or download pre-built binaries from:
https://github.com/lexxmark/winflexbison/releases

Extract and add to PATH.

### Option 2: Manual Installation

#### Install Abseil

```powershell
# Clone and build Abseil
git clone https://github.com/abseil/abseil-cpp.git
cd abseil-cpp
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=C:\abseil
cmake --build . --config Release
cmake --install .
```

#### Install Flex for Windows

Download from: https://github.com/lexxmark/winflexbison/releases

```powershell
# Extract to C:\winflexbison
# Add C:\winflexbison to PATH
```

#### Install Google Test (optional)

```powershell
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=C:\gtest
cmake --build . --config Release
cmake --install .
```

## Building with Visual Studio IDE

### Step 1: Open the Project

**Option A: Open Folder in Visual Studio (VS 2017+)**

1. Launch Visual Studio
2. File → Open → Folder
3. Navigate to `verible-preprocessor-standalone` folder
4. Click "Select Folder"
5. Visual Studio will automatically detect CMakeLists.txt

**Option B: Generate Solution File**

```powershell
cd verible-preprocessor-standalone
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
# This creates VerilogPreprocessorStandalone.sln
```

Then open the .sln file in Visual Studio.

### Step 2: Configure CMake (if using Open Folder)

Visual Studio will show CMake output in the Output window.

If you installed dependencies manually, configure CMake settings:

1. Project → CMake Settings
2. Add paths to dependencies:
   ```json
   {
     "configurations": [
       {
         "name": "x64-Release",
         "generator": "Ninja",
         "configurationType": "Release",
         "buildRoot": "${projectDir}\\out\\build\\${name}",
         "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=C:\\abseil;C:\\gtest",
         "buildCommandArgs": "",
         "ctestCommandArgs": ""
       }
     ]
   }
   ```

### Step 3: Build

- **Build Menu → Build All** (Ctrl+Shift+B)
- Or right-click on CMakeLists.txt → Build

Output will be in: `out\build\x64-Release\`

### Step 4: Run Examples and Tests

1. In Solution Explorer, expand "CMake Targets View"
2. Right-click on `preprocessor_example.exe` → Execute
3. Right-click on `preprocessor_tests.exe` → Execute

## Building from Command Line

### Using CMake and MSBuild

```powershell
# Open "x64 Native Tools Command Prompt for VS 2022"
# (or your Visual Studio version)

cd verible-preprocessor-standalone
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release

# Run examples
Release\preprocessor_example.exe

# Run tests
Release\preprocessor_tests.exe
```

### Using Ninja (faster builds)

```powershell
# Install Ninja
vcpkg install ninja:x64-windows

# Or download from https://github.com/ninja-build/ninja/releases

# Build
cd verible-preprocessor-standalone
mkdir build
cd build

cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja

# Run
.\preprocessor_example.exe
.\preprocessor_tests.exe
```

### Build Script

Use the provided `build.bat`:

```powershell
cd verible-preprocessor-standalone
.\build.bat
```

## Using vcpkg for Dependencies

### Toolchain File Method

If you used vcpkg, tell CMake about it:

```powershell
cd verible-preprocessor-standalone
mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

### Visual Studio Integration

If you ran `vcpkg integrate install`, Visual Studio will automatically find vcpkg packages. Just build normally.

## CMake Configuration Options

```powershell
# Disable tests
cmake .. -DBUILD_TESTS=OFF

# Disable examples
cmake .. -DBUILD_EXAMPLES=OFF

# Specify build type
cmake .. -DCMAKE_BUILD_TYPE=Release  # or Debug

# Install to specific location
cmake .. -DCMAKE_INSTALL_PREFIX=C:\my-preprocessor
cmake --build . --config Release
cmake --install . --config Release
```

## Troubleshooting

### Flex Not Found

**Error:** `Could not find FLEX executable`

**Solution:**
```powershell
# Option 1: Add to PATH
set PATH=%PATH%;C:\vcpkg\installed\x64-windows\tools\winflexbison

# Option 2: Specify in CMake
cmake .. -DFLEX_EXECUTABLE=C:\vcpkg\installed\x64-windows\tools\winflexbison\win_flex.exe
```

### Abseil Not Found

**Error:** `Could not find package absl`

**Solution:**
```powershell
# If using vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# If manual install
cmake .. -DCMAKE_PREFIX_PATH=C:\abseil
```

### LNK2019: Unresolved External Symbol

**Error:** Linker errors about Abseil symbols

**Solution:**
Ensure you're using the same runtime library:
```powershell
cmake .. -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
```

Or make sure vcpkg triplet matches (x64-windows uses dynamic linking).

### Flex Cannot Create Output File

**Error:** Permission denied or cannot create output file

**Solution:**
Run Visual Studio or Command Prompt as Administrator, or check folder permissions.

### C++17 Not Supported

**Error:** C++17 features not available

**Solution:**
Ensure you're using Visual Studio 2017 15.7 or later. Update CMake:
```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## Platform-Specific Notes

### Windows Path Separators

The library uses `std::string` for filenames, so both forward slashes and backslashes work:
```cpp
pp.SetIncludeFileCallback([](const std::string& file) {
    // Both work:
    // "includes\\common.vh"
    // "includes/common.vh"
    return LoadFile(file);
});
```

### Debug vs Release

Build both for development:
```powershell
# Debug (for development)
cmake --build . --config Debug

# Release (for production)
cmake --build . --config Release
```

Binaries will be in `Debug\` and `Release\` subdirectories.

## Integration with Visual Studio Projects

### Option 1: Add as Subdirectory

```cmake
# In your CMakeLists.txt
add_subdirectory(verible-preprocessor-standalone)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE verilog_preprocessor)
```

### Option 2: Install and Link

```powershell
# Build and install
cd verible-preprocessor-standalone\build
cmake --install . --prefix C:\my-libs

# In your project CMakeLists.txt
find_library(VERILOG_PREPROCESSOR
    NAMES verilog_preprocessor
    PATHS C:\my-libs\lib)

target_link_libraries(MyApp PRIVATE ${VERILOG_PREPROCESSOR})
target_include_directories(MyApp PRIVATE C:\my-libs\include)
```

### Option 3: Use as Static Library

After building, you can copy:
- `build\Release\verilog_preprocessor.lib` → Your project
- `api\verilog_preprocessor.h` → Your include directory

Then link manually in Visual Studio project settings.

## Performance Tips

1. **Use Ninja** instead of MSBuild for faster builds
2. **Enable parallel builds**: `/MP` flag (automatic with CMake)
3. **Use Release builds** for production (10-100x faster than Debug)
4. **Precompiled headers**: Automatically used when building with CMake

## Testing on Windows

```powershell
# Run tests
cd build
ctest -C Release

# Or run directly
Release\preprocessor_tests.exe

# With verbose output
Release\preprocessor_tests.exe --gtest_verbose
```

## Complete Example: Fresh Build on Windows

```powershell
# 1. Install vcpkg (one-time setup)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. Install dependencies (one-time)
.\vcpkg install abseil:x64-windows gtest:x64-windows winflexbison:x64-windows

# 3. Clone or extract the preprocessor library
cd C:\projects
# (assuming you have the code here)

# 4. Build
cd verible-preprocessor-standalone
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release

# 5. Test
Release\preprocessor_example.exe
Release\preprocessor_tests.exe

# 6. Install (optional)
cmake --install . --prefix C:\verilog-preprocessor
```

## Additional Resources

- **Visual Studio CMake Documentation**: https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio
- **vcpkg**: https://github.com/Microsoft/vcpkg
- **Win flex-bison**: https://github.com/lexxmark/winflexbison
- **Abseil**: https://abseil.io/docs/cpp/

## Getting Help

If you encounter issues:

1. Check the error message carefully
2. Verify all dependencies are installed
3. Ensure PATH includes Flex
4. Try building with verbose output: `cmake --build . --verbose`
5. Check the main README.md for general issues
