cmake_minimum_required(VERSION 3.16)
project(GertAutoClicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option for static builds
option(BUILD_STATIC "Build statically" OFF)

# Configure for static builds
if(BUILD_STATIC AND WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    add_definitions(-DQT_STATICPLUGIN -DQT_STATIC)
    
    # Force static linking
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MT /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Set up Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/autoclicker.cpp
    src/hotkeymanager.cpp
    src/clickerthread.cpp
)

set(HEADERS
    src/types.h
    src/mainwindow.h
    src/autoclicker.h
    src/hotkeymanager.h
    src/clickerthread.h
)

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core 
    Qt6::Widgets
    Qt6::Gui
)

# Platform-specific settings
if(WIN32)
    # Windows-specific libraries
    target_link_libraries(${PROJECT_NAME} user32)
    
    if(BUILD_STATIC)
        # Import static plugins
        qt6_import_plugins(${PROJECT_NAME} 
            INCLUDE Qt6::QWindowsIntegrationPlugin
        )
        
        # Additional Windows libraries required for static Qt
        target_link_libraries(${PROJECT_NAME} 
            # Qt dependencies
            dwmapi uxtheme wtsapi32 imm32 winmm
            ws2_32 iphlpapi crypt32 secur32 version
            setupapi oleaut32 ole32 shell32 advapi32
            kernel32 uuid gdi32 comdlg32 winspool
            netapi32 userenv
            # Direct3D and graphics
            d3d11 dxgi d3dcompiler
            # Additional Windows APIs
            shlwapi comctl32 wininet urlmon
        )
    endif()
    
    # Add icon resource if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icon.rc")
        target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/resources/icon.rc")
    endif()
    
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(CARBON_LIBRARY Carbon)
    find_library(APPSERVICES_LIBRARY ApplicationServices)
    target_link_libraries(${PROJECT_NAME} 
        ${COCOA_LIBRARY} 
        ${CARBON_LIBRARY}
        ${APPSERVICES_LIBRARY}
    )
    
elseif(UNIX)
    find_package(X11 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XTST REQUIRED xtst)
    
    target_link_libraries(${PROJECT_NAME} 
        ${X11_LIBRARIES} 
        ${X11_Xtst_LIB}
        ${XTST_LIBRARIES}
    )
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${X11_INCLUDE_DIR}
        ${XTST_INCLUDE_DIRS}
    )
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)