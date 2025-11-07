cmake_minimum_required(VERSION 3.15)
project(VerilogPreprocessorStandalone VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific settings
if(MSVC)
    # Enable parallel builds in MSVC
    add_compile_options(/MP)
    # Increase warning level
    add_compile_options(/W3)
    # Suppress specific warnings if needed
    add_compile_options(/wd4996)  # Disable deprecation warnings
    # Use UTF-8 encoding
    add_compile_options(/utf-8)
else()
    # GCC/Clang warnings
    add_compile_options(-Wall -Wextra)
endif()

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Find required packages
find_package(FLEX REQUIRED)
find_package(absl QUIET)

# If abseil not found, try to use system libraries
if(NOT absl_FOUND)
    message(STATUS "Abseil not found via find_package, assuming system installation")
    # We'll handle this with manual linking if needed
endif()

# Generate lexer from Flex file
set(FLEX_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/verible/verilog/parser/verilog.lex")
set(FLEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/verilog.lex.cc")

add_custom_command(
    OUTPUT "${FLEX_OUTPUT}"
    COMMAND ${FLEX_EXECUTABLE}
            --outfile="${FLEX_OUTPUT}"
            "${FLEX_INPUT}"
    DEPENDS "${FLEX_INPUT}"
    COMMENT "Generating lexer from verilog.lex"
)

# Main library - Verible core components
add_library(verible_core STATIC
    # Text utilities
    verible/common/text/token-info.cc
    verible/common/text/token-stream-view.cc
    verible/common/text/macro-definition.cc
    verible/common/text/text-structure.cc
    verible/common/text/concrete-syntax-tree.cc
    verible/common/text/concrete-syntax-leaf.cc
    verible/common/text/symbol.cc

    # Lexer infrastructure
    verible/common/lexer/token-stream-adapter.cc

    # String utilities
    verible/common/strings/line-column-map.cc

    # Verilog parser
    verible/verilog/parser/verilog-lexer.cc
    verible/verilog/parser/verilog-token.cc
    "${FLEX_OUTPUT}"

    # Verilog analysis
    verible/verilog/analysis/verilog-filelist.cc

    # Preprocessor
    verible/verilog/preprocessor/verilog-preprocess.cc
)

target_include_directories(verible_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link against abseil if available
if(absl_FOUND)
    # Check if absl::log exists (added in Abseil LTS 20230125)
    if(TARGET absl::log)
        set(ABSL_HAS_LOG TRUE)
    else()
        set(ABSL_HAS_LOG FALSE)
        message(STATUS "absl::log not found (requires Abseil >= 20230125), using fallback")
    endif()

    target_link_libraries(verible_core PUBLIC
        absl::status
        absl::statusor
        absl::strings
    )

    # Only link absl::log if available
    if(ABSL_HAS_LOG)
        target_link_libraries(verible_core PUBLIC absl::log)
    endif()
else()
    # Try to find abseil libraries manually
    find_library(ABSL_STATUS absl_status)
    find_library(ABSL_STATUSOR absl_statusor)
    find_library(ABSL_STRINGS absl_strings)
    find_library(ABSL_LOG absl_log_internal_check_op)

    if(ABSL_STATUS AND ABSL_STATUSOR AND ABSL_STRINGS)
        target_link_libraries(verible_core PUBLIC
            ${ABSL_STATUS}
            ${ABSL_STATUSOR}
            ${ABSL_STRINGS}
        )

        # Only add log if found
        if(ABSL_LOG)
            target_link_libraries(verible_core PUBLIC ${ABSL_LOG})
            set(ABSL_HAS_LOG TRUE)
        else()
            set(ABSL_HAS_LOG FALSE)
            message(STATUS "absl::log not found, using fallback")
        endif()
    else()
        message(WARNING "Abseil libraries not found. Build may fail.")
    endif()
endif()

# Standalone API wrapper library
add_library(verilog_preprocessor STATIC
    api/verilog_preprocessor.cc
)

target_include_directories(verilog_preprocessor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(verilog_preprocessor PUBLIC
    verible_core
)

# Install targets
install(TARGETS verilog_preprocessor verible_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES api/verilog_preprocessor.h
    DESTINATION include
)

# Examples
if(BUILD_EXAMPLES)
    add_executable(preprocessor_example
        examples/example.cc
    )

    target_link_libraries(preprocessor_example PRIVATE
        verilog_preprocessor
    )

    install(TARGETS preprocessor_example
        RUNTIME DESTINATION bin
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)

    if(GTest_FOUND)
        add_executable(preprocessor_tests
            tests/verilog_preprocessor_test.cc
        )

        target_link_libraries(preprocessor_tests PRIVATE
            verilog_preprocessor
            GTest::gtest
            GTest::gtest_main
        )

        add_test(NAME PreprocessorTests COMMAND preprocessor_tests)

        install(TARGETS preprocessor_tests
            RUNTIME DESTINATION bin
        )
    else()
        message(STATUS "GTest not found, skipping tests")
    endif()
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "  Abseil Found: ${absl_FOUND}")
if(ABSL_HAS_LOG)
    message(STATUS "  Abseil Log: YES (>= 20230125)")
else()
    message(STATUS "  Abseil Log: NO (using fallback CHECK macros)")
endif()
if(GTest_FOUND)
    message(STATUS "  GTest Found: YES")
else()
    message(STATUS "  GTest Found: NO")
endif()
message(STATUS "")
