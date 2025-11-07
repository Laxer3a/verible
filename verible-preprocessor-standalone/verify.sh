#!/bin/bash
# Verification script for standalone preprocessor library

echo "===================================================================="
echo "STANDALONE VERILOG PREPROCESSOR - VERIFICATION SCRIPT"
echo "===================================================================="
echo ""

# Check directory structure
echo "[1/5] Checking directory structure..."
REQUIRED_DIRS=("api" "verible" "examples" "tests")
MISSING_DIRS=0

for dir in "${REQUIRED_DIRS[@]}"; do
    if [ -d "$dir" ]; then
        echo "  ✓ $dir/"
    else
        echo "  ✗ $dir/ MISSING"
        MISSING_DIRS=$((MISSING_DIRS + 1))
    fi
done

# Check core files
echo ""
echo "[2/5] Checking core API files..."
CORE_FILES=(
    "api/verilog_preprocessor.h"
    "api/verilog_preprocessor.cc"
    "CMakeLists.txt"
    "README.md"
)

MISSING_FILES=0
for file in "${CORE_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file MISSING"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

# Check preprocessor core
echo ""
echo "[3/5] Checking Verible preprocessor core..."
PREPROC_FILES=(
    "verible/verilog/preprocessor/verilog-preprocess.h"
    "verible/verilog/preprocessor/verilog-preprocess.cc"
    "verible/verilog/parser/verilog-lexer.h"
    "verible/verilog/parser/verilog-lexer.cc"
    "verible/verilog/parser/verilog.lex"
)

for file in "${PREPROC_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ $file MISSING"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

# Count files
echo ""
echo "[4/5] Counting files..."
HEADER_COUNT=$(find verible api -name "*.h" 2>/dev/null | wc -l)
SOURCE_COUNT=$(find verible api -name "*.cc" 2>/dev/null | wc -l)
LEX_COUNT=$(find verible -name "*.lex" 2>/dev/null | wc -l)

echo "  Headers (.h):       $HEADER_COUNT"
echo "  Sources (.cc):      $SOURCE_COUNT"
echo "  Flex sources (.lex): $LEX_COUNT"
echo "  Total code files:   $((HEADER_COUNT + SOURCE_COUNT + LEX_COUNT))"

# Check for external references
echo ""
echo "[5/5] Checking for unwanted external dependencies..."
EXTERNAL_REFS=0

# Check if any files try to include from absolute verible paths
if grep -r "include.*\"verible/" verible/ 2>/dev/null | grep -v Binary > /tmp/check.txt; then
    echo "  ⚠ Found includes with verible/ prefix (this is expected)"
    EXTERNAL_REFS=$(cat /tmp/check.txt | wc -l)
else
    echo "  ✓ No unexpected external path references"
fi

# Summary
echo ""
echo "===================================================================="
echo "VERIFICATION SUMMARY"
echo "===================================================================="

if [ $MISSING_DIRS -eq 0 ] && [ $MISSING_FILES -eq 0 ]; then
    echo "✓ All required files and directories present"
    echo "✓ Project structure is complete"
    echo ""
    echo "STATUS: READY TO BUILD"
    echo ""
    echo "Next steps:"
    echo "  mkdir build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"
    exit 0
else
    echo "✗ INCOMPLETE: $MISSING_DIRS directories missing, $MISSING_FILES files missing"
    echo ""
    echo "STATUS: NOT READY"
    exit 1
fi
