#!/bin/bash

# Gert Auto Clicker - Windows Cross-Compilation Build Script
# This script builds the auto-clicker application for Windows from macOS/Linux
#
# NOTE: This requires Qt6 for Windows cross-compilation setup.
# For easier Windows builds, use GitHub Actions (.github/workflows/build-windows.yml)

set -e  # Exit on any error

BUILD_TYPE="Release"
CLEAN_BUILD=false

echo "🎯 Building Gert Auto Clicker for Windows from $(uname -s)..."
echo "💡 For easier Windows builds, push to GitHub and use Actions"

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -debug            Build in Debug mode"
    echo "  -release          Build in Release mode (default)"
    echo "  -clean            Clean build directory before building"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "Requirements:"
    echo "  - MinGW-w64 cross-compiler"
    echo "  - Qt6 for Windows (cross-compiled)"
    echo ""
    echo "Install dependencies on macOS:"
    echo "  brew install mingw-w64"
    echo "  # Then install Qt6 for Windows cross-compilation"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -release)
            BUILD_TYPE="Release"
            shift
            ;;
        -clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            echo "❌ Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Check for MinGW-w64
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "❌ Error: MinGW-w64 not found. Please install it:"
    echo ""
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "  macOS: brew install mingw-w64"
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "  Ubuntu/Debian: sudo apt-get install mingw-w64"
        echo "  CentOS/RHEL: sudo yum install mingw64-gcc-c++"
        echo "  Arch: sudo pacman -S mingw-w64-gcc"
    fi
    echo ""
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf build-windows
fi

# Create build directory
echo "📁 Creating build directory..."
mkdir -p build-windows
cd build-windows

# Set up toolchain file for cross-compilation
echo "📝 Creating CMake toolchain file..."
cat > toolchain-windows.cmake << 'EOF'
# CMake toolchain file for Windows cross-compilation

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Cross-compilation tools
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set Windows-specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
EOF

# Configure with CMake
echo "⚙️  Configuring with CMake for Windows cross-compilation..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=toolchain-windows.cmake \
    -DCMAKE_PREFIX_PATH="${QT_WINDOWS_PATH:-/usr/x86_64-w64-mingw32/qt6}"

# Check if cmake configuration was successful
if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    echo ""
    echo "💡 Make sure you have:"
    echo "   1. MinGW-w64 installed: x86_64-w64-mingw32-gcc"
    echo "   2. Qt6 for Windows cross-compilation"
    echo "   3. Set QT_WINDOWS_PATH environment variable if Qt6 is not in /usr/x86_64-w64-mingw32/qt6"
    echo ""
    echo "Example Qt6 installation:"
    echo "   # Download Qt6 for Windows and extract to /usr/x86_64-w64-mingw32/qt6"
    echo "   export QT_WINDOWS_PATH=/path/to/qt6-windows"
    echo ""
    exit 1
fi

# Get number of processors for parallel build
if [[ "$OSTYPE" == "darwin"* ]]; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=$(nproc 2>/dev/null || echo 1)
fi

# Build the project
echo "🔨 Building Windows executable with $NPROC cores..."
make -j$NPROC

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Windows cross-compilation completed successfully!"
echo ""
echo "🚀 Windows executable created at:"
echo "   $(pwd)/bin/GertAutoClicker.exe"
echo ""
echo "📦 To distribute, you may need to bundle Qt6 DLLs with the executable"
echo "🎉 Build completed for Windows platform!"