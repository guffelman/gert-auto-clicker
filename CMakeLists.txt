cmake_minimum_required(VERSION 3.16)
project(GertAutoClicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option for static builds
option(BUILD_STATIC "Build statically" OFF)

# Configure Qt for static builds
if(BUILD_STATIC AND WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    set(Qt6_STATIC ON)
    add_definitions(-DQT_STATICPLUGIN)
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# For static builds, find additional components
if(BUILD_STATIC AND WIN32)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
    qt6_import_plugins(${PROJECT_NAME} INCLUDE Qt6::QWindowsIntegrationPlugin Qt6::QWindowsVistaStylePlugin)
endif()

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
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt libraries
if(BUILD_STATIC AND WIN32)
    target_link_libraries(${PROJECT_NAME} 
        Qt6::Core 
        Qt6::Widgets
        Qt6::Gui
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        Qt6::Core 
        Qt6::Widgets
    )
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific libraries and settings
    target_link_libraries(${PROJECT_NAME} user32)
    
    # Static linking configuration for Windows
    if(BUILD_STATIC)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
        
        # Static runtime library
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        
        # Additional static libraries for Qt static builds
        target_link_libraries(${PROJECT_NAME} 
            dwmapi
            uxtheme
            wtsapi32
            imm32
            winmm
            ws2_32
            iphlpapi
            crypt32
            secur32
            version
            setupapi
            oleaut32
            ole32
            shell32
            advapi32
            kernel32
            uuid
            gdi32
            comdlg32
            winspool
        )
    else()
        # Set Windows subsystem for GUI application
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
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