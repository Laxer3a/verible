# Abseil Compatibility Notes

## Issue: Ubuntu/Debian Older Abseil Version

### Problem

Ubuntu 22.04 and similar distributions ship with an older version of Abseil that does not include the `absl::log` library. This library was added in **Abseil LTS 20230125**.

When building on systems with older Abseil, you may encounter errors like:
```
CMake Error: Target "verible_core" links to target "absl::log" but the target was not found.
```

### Solution

The standalone preprocessor now includes **automatic fallback support** for older Abseil versions.

#### How It Works

1. **CMakeLists.txt** checks if `absl::log` target exists
2. If found, it links against it
3. If not found, it skips the `absl::log` dependency
4. **logging.h** provides fallback CHECK macros using standard C++

#### Fallback Implementation

When `absl::log` is not available, the library provides these macros:

```cpp
CHECK(condition)           // Basic assertion
CHECK_EQ(a, b)            // Equal
CHECK_NE(a, b)            // Not equal
CHECK_LT(a, b)            // Less than
CHECK_LE(a, b)            // Less than or equal
CHECK_GT(a, b)            // Greater than
CHECK_GE(a, b)            // Greater than or equal
CHECK_NOTNULL(ptr)        // Null pointer check
LOG(severity)             // Basic logging to stderr
```

These macros provide the same functionality as Abseil's log library, just with simpler implementation using `std::cerr` and `std::abort()`.

## Supported Abseil Versions

| Abseil Version | Status | Notes |
|---------------|--------|-------|
| < 20230125 | ✅ Supported | Uses fallback CHECK macros |
| >= 20230125 | ✅ Supported | Uses native absl::log |

## Distribution Compatibility

| Distribution | Abseil Version | Status |
|-------------|---------------|--------|
| Ubuntu 22.04 | 20210324 | ✅ Works with fallback |
| Ubuntu 24.04 | 20240116+ | ✅ Works with native log |
| Debian 11 | 20200923 | ✅ Works with fallback |
| Debian 12 | 20230125+ | ✅ Works with native log |
| Fedora 38+ | 20230125+ | ✅ Works with native log |
| vcpkg | Latest | ✅ Works with native log |

## Build Output

When configuring, you'll see:

**With newer Abseil:**
```
-- Configuration Summary:
--   Abseil Found: 1
--   Abseil Log: YES (>= 20230125)
```

**With older Abseil:**
```
-- Configuration Summary:
--   Abseil Found: 1
--   Abseil Log: NO (using fallback CHECK macros)
```

## Manual Abseil Upgrade (Optional)

If you want to use the native `absl::log` library on Ubuntu 22.04, you can build Abseil from source:

```bash
# Clone Abseil
git clone https://github.com/abseil/abseil-cpp.git
cd abseil-cpp

# Checkout LTS version
git checkout 20240116.0

# Build and install
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DABSL_BUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

Then rebuild the preprocessor library.

## For vcpkg Users (Windows)

vcpkg always provides the latest Abseil, so you'll automatically get `absl::log` support:

```powershell
vcpkg install abseil:x64-windows
```

## Impact on Functionality

**None.** Both implementations provide identical functionality:
- ✅ All CHECK macros work the same way
- ✅ Same assertion behavior
- ✅ Same error messages
- ✅ Same program termination on check failure

The only difference is the implementation detail - native Abseil log vs. fallback using standard C++.

## Technical Details

The fallback is implemented using C++17 `__has_include` preprocessor feature:

```cpp
#if __has_include("absl/log/check.h")
  // Use native Abseil log
  #include "absl/log/check.h"
#else
  // Use fallback CHECK macros
  #define CHECK(condition) ...
#endif
```

This ensures compile-time detection and zero runtime overhead.

## Troubleshooting

### Still getting absl::log errors?

1. Clear your build directory:
   ```bash
   rm -rf build
   mkdir build && cd build
   ```

2. Reconfigure:
   ```bash
   cmake ..
   ```

3. Check the output - you should see:
   ```
   -- absl::log not found (requires Abseil >= 20230125), using fallback
   ```

### Want to verify which implementation is used?

Check your build output:
```bash
cmake .. 2>&1 | grep -i "abseil log"
```

You should see either:
- `Abseil Log: YES (>= 20230125)` - Using native
- `Abseil Log: NO (using fallback CHECK macros)` - Using fallback

## References

- [Abseil Releases](https://github.com/abseil/abseil-cpp/releases)
- [Abseil Log Library](https://abseil.io/docs/cpp/guides/log)
- [Ubuntu Abseil Package](https://packages.ubuntu.com/search?keywords=libabsl-dev)
