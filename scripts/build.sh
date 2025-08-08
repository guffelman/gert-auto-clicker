#!/bin/bash

# Gert Auto Clicker Build Script
# This script builds the auto-clicker application for multiple platforms

set -e  # Exit on any error

# Default values
BUILD_PLATFORM="auto"
BUILD_TYPE="Release"
CLEAN_BUILD=false

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -mac, --macos     Build for macOS (cross-compile if on different platform)"
    echo "  -win, --windows   Build for Windows (cross-compile if on different platform)"
    echo "  -linux            Build for Linux (cross-compile if on different platform)"
    echo "  -auto, --auto     Auto-detect platform (default)"
    echo "  -debug            Build in Debug mode"
    echo "  -release          Build in Release mode (default)"
    echo "  -clean            Clean build directory before building"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                 # Build for current platform"
    echo "  $0 -mac           # Build for macOS"
    echo "  $0 -windows       # Build for Windows"
    echo "  $0 -clean         # Clean build and build for current platform"
    echo "  $0 -mac -debug    # Build for macOS in debug mode"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -mac|--macos)
            BUILD_PLATFORM="macos"
            shift
            ;;
        -win|-windows|--windows)
            BUILD_PLATFORM="windows"
            shift
            ;;
        -linux)
            BUILD_PLATFORM="linux"
            shift
            ;;
        -auto|--auto)
            BUILD_PLATFORM="auto"
            shift
            ;;
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

# Auto-detect platform if not specified
if [ "$BUILD_PLATFORM" = "auto" ]; then
    if [[ "$OSTYPE" == "darwin"* ]]; then
        BUILD_PLATFORM="macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        BUILD_PLATFORM="windows"
    else
        BUILD_PLATFORM="linux"
    fi
fi

echo "🎯 Building Gert Auto Clicker for $BUILD_PLATFORM ($BUILD_TYPE)..."

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf build
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "📁 Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Platform-specific CMake configuration
echo "⚙️  Configuring with CMake for $BUILD_PLATFORM..."

case $BUILD_PLATFORM in
    "windows")
        # Windows cross-compilation or native build
        if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
            # Native Windows build
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        else
            # Cross-compilation for Windows
            echo "🔧 Setting up cross-compilation for Windows..."
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
                     -DCMAKE_SYSTEM_NAME=Windows \
                     -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
                     -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
                     -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32
        fi
        ;;
    "macos")
        # macOS cross-compilation or native build
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # Native macOS build
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        else
            # Cross-compilation for macOS (requires osxcross)
            echo "🔧 Setting up cross-compilation for macOS..."
            if [ -z "$OSXCROSS_ROOT" ]; then
                echo "❌ Error: OSXCROSS_ROOT environment variable not set for macOS cross-compilation"
                echo "   Please install osxcross and set OSXCROSS_ROOT"
                exit 1
            fi
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
                     -DCMAKE_SYSTEM_NAME=Darwin \
                     -DCMAKE_C_COMPILER=o64-clang \
                     -DCMAKE_CXX_COMPILER=o64-clang++ \
                     -DCMAKE_FIND_ROOT_PATH=$OSXCROSS_ROOT/target
        fi
        ;;
    "linux")
        # Linux cross-compilation or native build
        if [[ "$OSTYPE" != "darwin"* ]] && [[ "$OSTYPE" != "msys" ]] && [[ "$OSTYPE" != "cygwin" ]]; then
            # Native Linux build
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE
        else
            # Cross-compilation for Linux
            echo "🔧 Setting up cross-compilation for Linux..."
            cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
                     -DCMAKE_SYSTEM_NAME=Linux \
                     -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc \
                     -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++
        fi
        ;;
esac

# Check if cmake configuration was successful
if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Get number of processors for parallel build
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    NPROC=$(sysctl -n hw.ncpu)
else
    # Linux and other Unix-like systems
    NPROC=$(nproc 2>/dev/null || echo 1)
fi

# Build the project
echo "🔨 Building project with $NPROC cores..."
make -j$NPROC

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build completed successfully!"
echo ""

# Platform-specific output information
case $BUILD_PLATFORM in
    "windows")
        echo "🚀 To run the application:"
        echo "   .\\bin\\$BUILD_TYPE\\GertAutoClicker.exe"
        echo ""
        echo "📁 Executable location: $(pwd)/bin/$BUILD_TYPE/GertAutoClicker.exe"
        ;;
    "macos")
        echo "🚀 To run the application:"
        echo "   ./bin/GertAutoClicker"
        echo ""
        echo "📁 Executable location: $(pwd)/bin/GertAutoClicker"
        ;;
    "linux")
        echo "🚀 To run the application:"
        echo "   ./bin/GertAutoClicker"
        echo ""
        echo "📁 Executable location: $(pwd)/bin/GertAutoClicker"
        ;;
esac

echo ""
echo "🎉 Build completed for $BUILD_PLATFORM platform!" 