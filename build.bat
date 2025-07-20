@echo off
REM MiniDB Build Script for Windows

echo Building MiniDB...

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

REM Build the project
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Executable: Release\minidb.exe
echo Tests: tests\Release\minidb_tests.exe

REM Run tests
echo.
echo Running tests...
tests\Release\minidb_tests.exe
if %ERRORLEVEL% NEQ 0 (
    echo Some tests failed.
    exit /b 1
)

echo.
echo All tests passed!
echo MiniDB is ready to use!
echo Run 'Release\minidb.exe' to start the interactive CLI

cd ..
