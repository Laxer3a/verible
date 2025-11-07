# Abseil Version Compatibility

## Critical Requirement

**Minimum Version: Abseil LTS 20230125**

The standalone Verilog preprocessor **requires Abseil 20230125 or later**.

## Why This Specific Version?

Abseil 20230125 introduced several critical fixes:

1. **absl::log library** - While we provide fallback CHECK macros, other parts depend on modern Abseil
2. **Fixed StrCat** - Older versions don't support `std::string_view` properly  
3. **Standardized string_view** - Old versions use `absl::debian3::string_view` (incompatible)
4. **C++17 improvements** - Better modern C++ support

## Distribution Compatibility

| Distribution | Abseil Version | Status | Action Required |
|-------------|---------------|--------|-----------------|
| Ubuntu 22.04 | 20220623 | ❌ **TOO OLD** | Upgrade Abseil (see below) |
| Ubuntu 24.04 | 20240116+ | ✅ **Works** | Install package |
| Debian 11 | 20200923 | ❌ **TOO OLD** | Upgrade Abseil (see below) |
| Debian 12 | 20230125+ | ✅ **Works** | Install package |
| Fedora 38+ | 20230125+ | ✅ **Works** | Install package |
| macOS (Homebrew) | Latest | ✅ **Works** | Install with brew |
| Windows (vcpkg) | Latest | ✅ **Works** | Install with vcpkg |

## Upgrading Abseil on Ubuntu 22.04 / Debian 11

If you're on Ubuntu 22.04 or Debian 11, you'll need to build Abseil from source.

### Quick Instructions

```bash
# Download Abseil LTS 20240116
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
sudo ldconfig
```

Then build the preprocessor normally.

**See [DEPENDENCIES.md](DEPENDENCIES.md) for complete upgrade instructions.**

## What About the Fallback Support?

The library includes fallback CHECK macros when `absl::log` is not available:

```cpp
#if __has_include("absl/log/check.h")
  // Use native absl::log
#else
  // Use fallback CHECK macros
#endif
```

However, this **only** addresses the missing `absl::log` library. Other API incompatibilities in older Abseil versions will still cause compilation errors.

### Fallback Scope

**What the fallback covers:**
- ✅ CHECK macros (CHECK, CHECK_EQ, etc.)
- ✅ CHECK_NOTNULL
- ✅ Basic LOG macros

**What requires Abseil 20230125+:**
- ❌ StrCat with std::string_view
- ❌ string_view type compatibility
- ❌ Modern string utilities
- ❌ Other Abseil improvements

## Build Output

When you configure, you'll see which version you have:

### With Abseil >= 20230125:
```
-- Configuration Summary:
--   Abseil Found: 1
--   Abseil Log: YES (>= 20230125)
```

### With Abseil < 20230125 but >= 20230125 (using fallback):
```
-- Configuration Summary:
--   Abseil Found: 1
--   Abseil Log: NO (using fallback CHECK macros)
```

### With Abseil < 20220000 (will fail):
```
-- Configuration Summary:
--   Abseil Found: 1
--   Abseil Log: NO (using fallback CHECK macros)

[Various compilation errors will follow]
```

## Errors You'll See with Old Abseil

### Error 1: string_view conversion
```
error: could not convert from 'std::string_view' to 'absl::debian3::string_view'
```

**Cause:** Old Abseil uses `absl::debian3::string_view` namespace  
**Solution:** Upgrade to Abseil 20230125+

### Error 2: StrCat with string_view
```
error: no matching function for call to 'StrCat(...)'
note: cannot convert 'std::string_view' to 'const absl::AlphaNum&'
```

**Cause:** Old Abseil's StrCat doesn't accept std::string_view  
**Solution:** Upgrade to Abseil 20230125+

### Error 3: Missing absl::log
```
CMake Error: Target links to "absl::log" but the target was not found
```

**Cause:** Missing absl::log library  
**Solution:** Either upgrade to Abseil 20230125+ or rely on fallback (but see other errors)

## Alternative: Use Ubuntu 24.04

The easiest solution is to use Ubuntu 24.04 or later, which includes a compatible Abseil version:

```bash
# Ubuntu 24.04
sudo apt-get install cmake g++ flex libabsl-dev libgtest-dev

# Build normally
cd verible-preprocessor-standalone
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## WSL2 Users

If you're using WSL2 with Ubuntu 22.04, you'll need to upgrade Abseil using the instructions above.

The WSL Ubuntu distribution has the same Abseil version as native Ubuntu.

## Testing Your Abseil Version

Check your installed Abseil version:

```bash
# Ubuntu/Debian
dpkg -l | grep libabsl

# Or check CMake detection
cd verible-preprocessor-standalone/build
cmake .. 2>&1 | grep -i abseil
```

## References

- [DEPENDENCIES.md](DEPENDENCIES.md) - Complete dependency upgrade guide
- [Abseil Releases](https://github.com/abseil/abseil-cpp/releases)
- [Abseil LTS Policy](https://abseil.io/about/releases)
