cmake_minimum_required(VERSION 3.14)  # Adjust CMake version as needed
project(delphinos-installer LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Automoc 
set(CMAKE_AUTOMOC ON)

# Find Qt and required components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Multimedia MultimediaWidgets)

# Find GLib and GIO using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)

# Find KPMcore
find_package(KPMcore REQUIRED)

# Add executable
set(SOURCES
    main.cpp
    mainWindow.cpp
    localizationPage.cpp
    networkDBus.cpp
    networkPage.cpp
    partitionPage.cpp
    installationPage.cpp
    usersPage.cpp
)

set(HEADERS
    mainWindow.hpp
    statusIndicator.hpp
    localizationPage.hpp
    networkDBus.hpp
    networkPage.hpp
    partitionPage.hpp
    installationPage.hpp
    usersPage.hpp
)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(TARGET_DIR ${CMAKE_BINARY_DIR})


add_executable(delphinos-installer ${SOURCES} ${HEADERS})

add_custom_target(copy_system_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/systemInstallation ${TARGET_DIR}/systemInstallation
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/statusIndicator ${TARGET_DIR}/statusIndicator
    COMMENT "Copying system installation scripts and status indicator"
)

add_dependencies(delphinos-installer copy_system_files)

# Include directories
target_include_directories(delphinos-installer PRIVATE
    ${GLIB_INCLUDE_DIRS}
    /usr/include/libnm
    /usr/include/kpmcore
)

# Link libraries
target_link_libraries(delphinos-installer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ${GLIB_LIBRARIES}
    kpmcore
)

# Install the executable (optional)
install(TARGETS delphinos-installer DESTINATION build)