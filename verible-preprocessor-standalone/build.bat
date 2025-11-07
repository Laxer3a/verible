@echo off
REM Build script for Verilog Preprocessor on Windows
REM Usage: build.bat [Release|Debug] [clean]

setlocal enabledelayedexpansion

echo ============================================================
echo Standalone Verilog Preprocessor - Windows Build Script
echo ============================================================
echo.

REM Parse command line arguments
set BUILD_TYPE=Release
set CLEAN_BUILD=0

:parse_args
if "%~1"=="" goto :done_parsing
if /i "%~1"=="Debug" set BUILD_TYPE=Debug
if /i "%~1"=="Release" set BUILD_TYPE=Release
if /i "%~1"=="clean" set CLEAN_BUILD=1
shift
goto :parse_args
:done_parsing

echo Build Type: %BUILD_TYPE%
echo.

REM Check for Visual Studio
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Visual Studio compiler not found!
    echo.
    echo Please run this script from:
    echo   - "x64 Native Tools Command Prompt for VS 2022" ^(or your VS version^)
    echo   - Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    echo.
    pause
    exit /b 1
)

REM Check for CMake
where cmake.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found!
    echo Please install CMake from https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

REM Detect vcpkg if available
set VCPKG_TOOLCHAIN=
if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
    echo Found vcpkg toolchain: C:\vcpkg
    echo.
)

REM Clean build if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist build (
        rmdir /s /q build
    )
    echo.
)

REM Create build directory
if not exist build (
    mkdir build
)

cd build

echo ============================================================
echo Configuring CMake...
echo ============================================================
echo.

cmake .. %VCPKG_TOOLCHAIN% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Common issues:
    echo   1. Abseil not found - Install via: vcpkg install abseil:x64-windows
    echo   2. Flex not found - Install via: vcpkg install winflexbison:x64-windows
    echo   3. GTest not found ^(optional^) - Install via: vcpkg install gtest:x64-windows
    echo.
    echo For detailed instructions, see WINDOWS_BUILD.md
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo Building...
echo ============================================================
echo.

cmake --build . --config %BUILD_TYPE% -- /maxcpucount
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================================
echo Build Successful!
echo ============================================================
echo.

REM List output files
echo Output files:
if exist "%BUILD_TYPE%\verilog_preprocessor.lib" (
    echo   [LIB] %BUILD_TYPE%\verilog_preprocessor.lib
)
if exist "%BUILD_TYPE%\preprocessor_example.exe" (
    echo   [EXE] %BUILD_TYPE%\preprocessor_example.exe
)
if exist "%BUILD_TYPE%\preprocessor_tests.exe" (
    echo   [EXE] %BUILD_TYPE%\preprocessor_tests.exe
)
echo.

REM Ask to run example
set /p RUN_EXAMPLE="Run example? (Y/N): "
if /i "%RUN_EXAMPLE%"=="Y" (
    if exist "%BUILD_TYPE%\preprocessor_example.exe" (
        echo.
        echo ============================================================
        echo Running Example
        echo ============================================================
        echo.
        %BUILD_TYPE%\preprocessor_example.exe
    ) else (
        echo Example executable not found.
    )
)

echo.
set /p RUN_TESTS="Run tests? (Y/N): "
if /i "%RUN_TESTS%"=="Y" (
    if exist "%BUILD_TYPE%\preprocessor_tests.exe" (
        echo.
        echo ============================================================
        echo Running Tests
        echo ============================================================
        echo.
        %BUILD_TYPE%\preprocessor_tests.exe
    ) else (
        echo Tests executable not found. Build with -DBUILD_TESTS=ON
    )
)

echo.
echo ============================================================
echo Build Complete!
echo ============================================================
echo.
echo Next steps:
echo   - Run example: build\%BUILD_TYPE%\preprocessor_example.exe
echo   - Run tests:   build\%BUILD_TYPE%\preprocessor_tests.exe
echo   - Install:     cmake --install . --prefix C:\verilog-preprocessor
echo.

cd ..

endlocal
