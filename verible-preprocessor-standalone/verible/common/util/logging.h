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

#ifndef VERIBLE_COMMON_UTIL_LOGGING_H_
#define VERIBLE_COMMON_UTIL_LOGGING_H_

// Check if we have the new absl::log library (Abseil >= 20230125)
// by checking for the existence of the check.h header
#if __has_include("absl/log/check.h")

// Use new Abseil logging
#ifdef __GNUC__
// b/246413374
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#include "absl/log/check.h"  // IWYU pragma: export
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "absl/log/die_if_null.h"  // IWYU pragma: export
#include "absl/log/log.h"          // IWYU pragma: export

#define CHECK_NOTNULL(p) (void)ABSL_DIE_IF_NULL(p)

#else

// Fallback for older Abseil versions (pre-20230125)
// Provide basic CHECK macros using assert
#include <cassert>
#include <cstdlib>
#include <iostream>

// Basic CHECK macro
#define CHECK(condition) \
  do { \
    if (!(condition)) { \
      std::cerr << "CHECK failed: " << #condition << std::endl; \
      std::abort(); \
    } \
  } while (0)

// CHECK with comparison operators
#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))

// CHECK_NOTNULL
#define CHECK_NOTNULL(p) \
  do { \
    if ((p) == nullptr) { \
      std::cerr << "CHECK_NOTNULL failed: " #p " is null" << std::endl; \
      std::abort(); \
    } \
  } while (0)

// LOG macros (basic implementation)
#define LOG(severity) std::cerr
#define VLOG(level) std::cerr

#endif  // __has_include("absl/log/check.h")

#endif  // VERIBLE_COMMON_UTIL_LOGGING_H_
