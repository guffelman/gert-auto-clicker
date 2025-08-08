@echo off
REM Gert Auto Clicker - Static Build Script for Windows
REM This script builds a completely standalone, portable Windows executable

echo 🎯 Building STATIC Gert Auto Clicker for Windows...
echo 📦 This will create a portable, standalone executable

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo ❌ Error: CMakeLists.txt not found. Please run this script from the project root.
    exit /b 1
)

REM Display Qt6 requirements
echo.
echo 📋 REQUIREMENTS FOR STATIC BUILD:
echo    - Qt6 with static libraries installed
echo    - MSVC 2019/2022 or MinGW-w64 compiler
echo    - Static Qt6 path: C:\Qt\6.x.x\msvc2019_64_static
echo.

REM Check for static Qt6 installation
set QT_STATIC_FOUND=0
if exist "C:\Qt\6.6.0\msvc2019_64_static" (
    set QT_STATIC_DIR=C:\Qt\6.6.0\msvc2019_64_static
    set QT_STATIC_FOUND=1
    echo ✅ Found Qt6 Static: %QT_STATIC_DIR%
)
if exist "C:\Qt\6.7.0\msvc2019_64_static" (
    set QT_STATIC_DIR=C:\Qt\6.7.0\msvc2019_64_static
    set QT_STATIC_FOUND=1
    echo ✅ Found Qt6 Static: %QT_STATIC_DIR%
)
if exist "C:\Qt\6.8.0\msvc2019_64_static" (
    set QT_STATIC_DIR=C:\Qt\6.8.0\msvc2019_64_static
    set QT_STATIC_FOUND=1
    echo ✅ Found Qt6 Static: %QT_STATIC_DIR%
)

if %QT_STATIC_FOUND%==0 (
    echo ⚠️  Warning: Static Qt6 not found in standard locations
    echo    Looking for: C:\Qt\6.x.x\msvc2019_64_static
    echo    You may need to specify CMAKE_PREFIX_PATH manually
    echo.
)

REM Clean build directory
if exist "build-static" (
    echo 🧹 Cleaning previous static build...
    rmdir /s /q build-static
)

REM Create build directory
echo 📁 Creating static build directory...
mkdir build-static
cd build-static

REM Configure with CMake for static build
echo ⚙️  Configuring CMake for STATIC Qt6 build...
if %QT_STATIC_FOUND%==1 (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DCMAKE_PREFIX_PATH="%QT_STATIC_DIR%"
) else (
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON
)

REM Check if cmake was successful
if errorlevel 1 (
    echo ❌ CMake configuration failed!
    echo.
    echo 💡 Troubleshooting:
    echo    1. Install Qt6 with static libraries
    echo    2. Use Qt Maintenance Tool to add 'Qt 6.x.x Static Library'
    echo    3. Ensure MSVC 2019/2022 is installed
    echo    4. Run this from 'Developer Command Prompt for VS'
    echo.
    exit /b 1
)

REM Build the project
echo 🔨 Building STATIC executable...
cmake --build . --config Release --parallel

REM Check if build was successful
if errorlevel 1 (
    echo ❌ Static build failed!
    exit /b 1
)

echo ✅ STATIC BUILD COMPLETED SUCCESSFULLY!
echo.
echo 🚀 Standalone executable created at:
echo    %cd%\bin\Release\GertAutoClicker.exe
echo.
echo 📦 This executable is COMPLETELY PORTABLE and requires NO additional DLLs!
echo 🎉 You can copy this single .exe file to any Windows machine and it will run!
echo.

REM Verify the executable exists
if exist "bin\Release\GertAutoClicker.exe" (
    echo 📊 File size information:
    dir "bin\Release\GertAutoClicker.exe" | findstr GertAutoClicker.exe
    echo.
    echo 🔍 Dependency check (should show minimal Windows system DLLs only):
    where dumpbin >nul 2>nul
    if not errorlevel 1 (
        dumpbin /dependents "bin\Release\GertAutoClicker.exe" | findstr "\.dll"
    ) else (
        echo    Note: Install Visual Studio for dependency analysis with 'dumpbin'
    )
) else (
    echo ⚠️  Warning: Expected executable not found at bin\Release\GertAutoClicker.exe
)

echo.
pause