@echo off
REM Gert Auto Clicker Build Script for Windows
REM This script builds the auto-clicker application on Windows

echo 🎯 Building Gert Auto Clicker for Windows...

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ❌ Error: CMakeLists.txt not found. Please run this script from the project root.
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "build" (
    echo 📁 Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Configure with CMake
echo ⚙️  Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Check if cmake was successful
if errorlevel 1 (
    echo ❌ CMake configuration failed!
    exit /b 1
)

REM Build the project
echo 🔨 Building project...
cmake --build . --config Release

REM Check if build was successful
if errorlevel 1 (
    echo ❌ Build failed!
    exit /b 1
)

echo ✅ Build completed successfully!
echo.
echo 🚀 To run the application:
echo    bin\Release\GertAutoClicker.exe
echo.
echo 📁 Executable location: %cd%\bin\Release\GertAutoClicker.exe

pause 