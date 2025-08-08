# Gert Auto Clicker

A modern, cross-platform auto-clicker application built with Qt6 and C++. Features a beautiful dark-themed GUI with hotkey support, customizable click intervals, and system tray integration.

## Features

- **Cross-Platform Support**: Works on Windows, macOS, and Linux
- **Modern Dark UI**: Beautiful, modern interface with dark theme
- **Hotkey Support**: Global hotkeys to start/stop clicking (F6 by default)
- **Multiple Click Types**: Left, right, middle, and double-click support
- **Customizable Intervals**: Adjustable click intervals from 1ms to 10 seconds
- **Click Modes**: Continuous, limited, and while-pressed modes
- **System Tray Integration**: Minimize to system tray with context menu
- **Settings Persistence**: Remembers your preferences between sessions
- **Real-time Status**: Live click counter and status updates

## Screenshots

The application features a clean, modern interface with:
- Click settings panel with interval, type, and mode controls
- Start/Stop button with visual feedback
- Minimize to tray functionality
- Real-time click counter
- Status display

## Building

### Prerequisites

- CMake 3.16 or higher
- Qt6 (Core and Widgets components)
- C++17 compatible compiler

#### Cross-Compilation Requirements

For building on one platform for another platform:

**Windows Cross-Compilation:**
- MinGW-w64 toolchain: `sudo apt install mingw-w64` (Ubuntu/Debian)
- Or install MinGW-w64 on your system

**macOS Cross-Compilation:**
- osxcross toolchain (requires Xcode command line tools)
- Set `OSXCROSS_ROOT` environment variable

**Linux Cross-Compilation:**
- Standard GCC toolchain for target architecture

### Dependencies

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev libx11-dev libxtst-dev
```

#### macOS:
```bash
brew install cmake qt6
```

#### Windows:
- Install Visual Studio 2019 or later with C++ support
- Install Qt6 from the official installer
- Install CMake

### Build Instructions

#### Using Build Scripts (Recommended)

**Linux/macOS:**
```bash
git clone <repository-url>
cd Gert-Auto-Clicker
chmod +x scripts/build.sh

# Build for current platform
./scripts/build.sh

# Build for specific platform
./scripts/build.sh -mac          # Build for macOS
./scripts/build.sh -windows      # Build for Windows (cross-compile)
./scripts/build.sh -linux        # Build for Linux

# Additional options
./scripts/build.sh -clean        # Clean build directory first
./scripts/build.sh -debug        # Build in debug mode
./scripts/build.sh -mac -debug   # Build for macOS in debug mode
./scripts/build.sh --help        # Show all options
```

**Windows:**
```cmd
git clone <repository-url>
cd Gert-Auto-Clicker
scripts\build.bat
```

**Dedicated Windows Cross-Compilation:**
```bash
./scripts/build-windows.sh       # Advanced cross-compilation setup
```

#### Manual Build

1. Clone the repository:
```bash
git clone <repository-url>
cd Gert-Auto-Clicker
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Configure and build:
```bash
# Linux/macOS
cmake ..
make -j$(nproc)

# Windows
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

4. Run the application:
```bash
# Linux/macOS
./bin/GertAutoClicker

# Windows
.\bin\Release\GertAutoClicker.exe
```

## Usage

1. **Start the Application**: Launch Gert Auto Clicker
2. **Configure Settings**:
   - Set click interval (milliseconds)
   - Choose click type (left, right, middle, double)
   - Select click mode (continuous, limited, while pressed)
   - Change hotkey if desired
3. **Start Clicking**: Press the Start button or use the hotkey (F6)
4. **Monitor**: Watch the click counter and status
5. **Stop**: Press Stop or use the hotkey again
6. **Minimize**: Use the "Minimize to Tray" button to hide the window

## Hotkeys

- **F6** (default): Toggle start/stop clicking
- **F7-F12**: Alternative hotkeys (cycle through by clicking the hotkey button)

## Click Modes

- **Continuous**: Clicks indefinitely until stopped
- **Limited**: Clicks a specified number of times
- **While Pressed**: Clicks only while the hotkey is held down

## Platform-Specific Notes

### Windows
- Requires administrator privileges for global hotkeys
- Uses Windows API for mouse input simulation
- Built with Visual Studio 2019+ or MinGW-w64
- Qt6 must be properly installed and configured in PATH
- For best performance, run as administrator when using global hotkeys

### macOS
- Requires accessibility permissions in System Preferences
- Uses Core Graphics for mouse input simulation

### Linux
- Requires X11 and XTest extension
- May need additional permissions for global hotkeys

## Security and Permissions

This application requires system-level permissions to:
- Simulate mouse clicks
- Register global hotkeys
- Access system tray

These permissions are necessary for the auto-clicker functionality and are standard for applications of this type.

## Development

### Project Structure

```
├── .github/workflows/       # CI/CD workflows
├── scripts/                 # Build scripts
│   ├── build.sh            # Multi-platform build script
│   ├── build.bat           # Windows batch build script
│   └── build-windows.sh    # Windows cross-compilation script
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.h/cpp   # Main window UI and logic
│   ├── autoclicker.h/cpp  # Core auto-clicker functionality
│   ├── hotkeymanager.h/cpp# Global hotkey management
│   ├── clickerthread.h/cpp# Platform-specific mouse clicking
│   └── types.h           # Common type definitions
├── CMakeLists.txt         # CMake build configuration
└── README.md             # This file
```

### Adding New Features

1. **New Click Types**: Add to the `ClickType` enum and implement in platform-specific functions
2. **New Click Modes**: Extend the `ClickMode` enum and add logic in `AutoClicker`
3. **UI Enhancements**: Modify `MainWindow` for new controls
4. **Platform Support**: Add platform-specific code in the respective files

## License

This project is provided as-is for educational and personal use. Please ensure compliance with your local laws and regulations regarding automation software.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Disclaimer

This software is provided for educational and legitimate automation purposes only. Users are responsible for ensuring they comply with all applicable laws and terms of service when using this software. 