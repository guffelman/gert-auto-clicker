cmake_minimum_required(VERSION 3.16)
project(GertAutoClicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option for static builds
option(BUILD_STATIC "Build statically" OFF)

# Set up Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configure for static builds
if(BUILD_STATIC AND WIN32)
    # Static linking configuration
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    
    # Define static plugin macros
    add_compile_definitions(QT_STATICPLUGIN QT_STATIC)
    
    # Static linking flags for MSVC
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
    
    # Static linking flags for MinGW
    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    endif()
    
    # Find Qt6 with static configuration - try common static Qt paths
    if(EXISTS "C:/Qt6Static/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt6Static/lib/cmake/Qt6")
    elseif(EXISTS "C:/Qt/6.6.0/msvc2019_64_static/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/6.6.0/msvc2019_64_static/lib/cmake/Qt6")
    elseif(EXISTS "C:/Qt/6.7.0/msvc2019_64_static/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/6.7.0/msvc2019_64_static/lib/cmake/Qt6")
    endif()
    
    # Force finding static libraries
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Enable Qt features for static builds
if(BUILD_STATIC AND WIN32)
    # Import static plugins for complete standalone operation
    qt6_import_plugins(${PROJECT_NAME}
        INCLUDE 
            Qt6::QWindowsIntegrationPlugin
            Qt6::QWindowsVistaStylePlugin
            Qt6::QModernWindowsStylePlugin
        EXCLUDE_BY_TYPE
            bearer
            networkinformation
    )
endif()

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
        # Additional Windows libraries required for static Qt
        target_link_libraries(${PROJECT_NAME} 
            # Core Windows libraries
            kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 
            ole32 oleaut32 uuid odbc32 odbccp32
            # Qt static dependencies  
            dwmapi uxtheme wtsapi32 imm32 winmm ws2_32 iphlpapi 
            crypt32 secur32 version setupapi netapi32 userenv
            # Graphics and input
            opengl32 glu32
        )
        
        # Set target properties for static build
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
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