# Dependency Requirements

## Summary

| Dependency | Minimum Version | Recommended | Ubuntu Package | Notes |
|-----------|----------------|-------------|----------------|-------|
| CMake | 3.15 | 3.20+ | cmake | Build system |
| C++ Compiler | C++17 | C++17 | g++ / clang++ | GCC 7+, Clang 5+, MSVC 2017+ |
| Flex | 2.5 | 2.6+ | flex | Lexer generator (build-time only) |
| **Abseil** | **20230125** | **Latest** | **See below** | **Critical - older versions incompatible** |
| Google Test | Any | 1.10+ | libgtest-dev | Optional, for tests only |

## Critical: Abseil Version Requirement

### Minimum Version: Abseil LTS 20230125

The preprocessor library **requires Abseil LTS 20230125 or later**.

Older versions (including those in Ubuntu 22.04) have API incompatibilities:
- ❌ **20220623** (Ubuntu 22.04) - **TOO OLD - Will not compile**
- ❌ **20210324** (Debian 11) - **TOO OLD - Will not compile**
- ✅ **20230125** - **Minimum required version**
- ✅ **20240116+** - Recommended

### Why This Version?

Abseil 20230125 introduced several critical features:
1. `absl::log` library (with fallback support in our code)
2. Fixed `StrCat` to properly support `std::string_view`
3. Standardized string_view usage (not `absl::debian3::string_view`)
4. Improved C++17 compatibility

## Per-Distribution Solutions

### Ubuntu 22.04 / Debian 11 (Has OLD Abseil)

The packaged Abseil is **too old**. You have three options:

#### Option 1: Build Abseil from Source (Recommended)

```bash
# Install prerequisites
sudo apt-get install cmake g++

# Download and build Abseil LTS 20240116
wget https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.2.tar.gz
tar xzf 20240116.2.tar.gz
cd abseil-cpp-20240116.2

# Build and install
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DABSL_BUILD_TESTING=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install

# Update library cache
sudo ldconfig
```

Then build the preprocessor normally:
```bash
cd verible-preprocessor-standalone
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Option 2: Use vcpkg

```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg
/opt/vcpkg/bootstrap-vcpkg.sh

# Install Abseil
/opt/vcpkg/vcpkg install abseil

# Build preprocessor with vcpkg
cd verible-preprocessor-standalone
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake
make -j$(nproc)
```

#### Option 3: Upgrade to Ubuntu 24.04

Ubuntu 24.04 includes a compatible Abseil version:
```bash
sudo do-release-upgrade
sudo apt-get install libabsl-dev
```

### Ubuntu 24.04 / Debian 12 (Has NEW Abseil)

Works out of the box! ✅

```bash
sudo apt-get install cmake g++ flex libabsl-dev libgtest-dev
cd verible-preprocessor-standalone
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Fedora 38+

Works out of the box! ✅

```bash
sudo dnf install cmake gcc-c++ flex abseil-cpp-devel gtest-devel
cd verible-preprocessor-standalone
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

Use Homebrew (provides latest Abseil):

```bash
brew install cmake flex abseil googletest
cd verible-preprocessor-standalone
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

### Windows

Use vcpkg (always provides latest):

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat

# Install dependencies
C:\vcpkg\vcpkg install abseil:x64-windows winflexbison:x64-windows

# Build
cd verible-preprocessor-standalone
.\build.bat
```

## Checking Your Abseil Version

### On Ubuntu/Debian:

```bash
dpkg -l | grep libabsl
# Look for version number in output
```

Or check the version in CMake output:
```bash
cd verible-preprocessor-standalone/build
cmake .. | grep -i abseil
```

### From Source:

Check the installed version:
```bash
pkg-config --modversion absl_base
```

Or check git tag if building from source:
```bash
cd abseil-cpp
git describe --tags
```

## WSL2 Users (Windows Subsystem for Linux)

If you're running Ubuntu 22.04 in WSL2, you'll need to build Abseil from source (Option 1 above).

The WSL Ubuntu distribution includes the same old Abseil as native Ubuntu 22.04.

## Docker/Container Users

Use Ubuntu 24.04 or later as your base image:

```dockerfile
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    cmake g++ flex libabsl-dev libgtest-dev

COPY verible-preprocessor-standalone /build
WORKDIR /build
RUN mkdir build && cd build && cmake .. && make -j$(nproc)
```

## Detailed Dependency Info

### CMake (3.15+)

Required for building. Available in all modern distributions.

**Check version:**
```bash
cmake --version
```

### C++17 Compiler

- **GCC 7+** (Ubuntu 18.04+)
- **Clang 5+** (Ubuntu 18.04+)
- **MSVC 2017+** (Windows)

**Check version:**
```bash
g++ --version
clang++ --version
```

### Flex (2.5+)

Lexical analyzer generator. Used at build time to generate the Verilog lexer.

**Not needed at runtime** - only during build.

**Check version:**
```bash
flex --version
```

### Google Test (Optional)

Only needed if building tests (`-DBUILD_TESTS=ON`).

Can be disabled:
```bash
cmake .. -DBUILD_TESTS=OFF
```

## Troubleshooting

### "Could not find absl::log" Error

This means you have Abseil < 20230125. The library provides fallback macros, but you may still hit other compatibility issues.

**Solution:** Upgrade to Abseil 20230125+

### "string_view incompatibility" Errors

This indicates a very old Abseil with `absl::debian3::string_view` instead of standard types.

**Solution:** Upgrade to Abseil 20230125+

### "StrCat" Errors with Multiple Arguments

Old Abseil versions don't support variadic `StrCat` with `std::string_view`.

**Solution:** Upgrade to Abseil 20230125+

### CMake Can't Find Abseil After Building from Source

Make sure to update the library cache:
```bash
sudo ldconfig
```

And check that the install prefix matches:
```bash
cmake .. -DCMAKE_PREFIX_PATH=/usr/local
```

### vcpkg Not Finding Libraries

Make sure to use the toolchain file:
```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Quick Reference

### Minimum Viable Build (Ubuntu 24.04+)

```bash
sudo apt-get install cmake g++ flex libabsl-dev
cd verible-preprocessor-standalone && mkdir build && cd build
cmake .. && make
```

### Minimum Viable Build (Ubuntu 22.04)

```bash
# Build Abseil first (one-time)
wget https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.2.tar.gz
tar xzf 20240116.2.tar.gz && cd abseil-cpp-20240116.2
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DABSL_BUILD_TESTING=OFF
sudo make install && sudo ldconfig

# Then build preprocessor
cd verible-preprocessor-standalone && mkdir build && cd build
cmake .. && make
```

## References

- [Abseil Releases](https://github.com/abseil/abseil-cpp/releases)
- [Abseil LTS Branches](https://abseil.io/about/releases)
- [vcpkg](https://github.com/Microsoft/vcpkg)
- [Ubuntu Packages](https://packages.ubuntu.com/)
