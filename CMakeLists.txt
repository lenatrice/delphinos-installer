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

# Add executable for delphinos-installer-elevated
set(SOURCES
    delphinosInstallerElevated.cpp
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

add_executable(delphinos-installer main.cpp)
add_executable(delphinos-installer-elevated ${SOURCES} ${HEADERS})

add_custom_target(copy_system_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/systemInstallation ${TARGET_DIR}/systemInstallation
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR}/statusIndicator ${TARGET_DIR}/statusIndicator
    COMMENT "Copying system installation scripts and status indicator"
)

add_dependencies(delphinos-installer-elevated copy_system_files)

# Include directories for the elevated executable
target_include_directories(delphinos-installer-elevated PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${POLKIT_INCLUDE_DIRS}
    /usr/include/kpmcore
)

# Link libraries for both executables
target_link_libraries(delphinos-installer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${GLIB_LIBRARIES}
    ${POLKIT_LIBRARIES}
)

target_link_libraries(delphinos-installer-elevated PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    ${GLIB_LIBRARIES}
    kpmcore
)

# Calculate the checksums of the script before compiling
find_program(SHA256SUM "shasum")

if(SHA256SUM)
    message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

    execute_process(
        COMMAND ${SHA256SUM} -a 256 ${CMAKE_SOURCE_DIR}/systemInstallation/systemInstallation.sh
        RESULT_VARIABLE result
        OUTPUT_VARIABLE checksumSystemInstallation
        ERROR_VARIABLE error
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    string(REGEX REPLACE " .*" "" checksumSystemInstallation "${checksumSystemInstallation}")

    execute_process(
        COMMAND ${SHA256SUM} -a 256 ${CMAKE_SOURCE_DIR}/systemInstallation/installPackages.sh
        RESULT_VARIABLE result
        OUTPUT_VARIABLE checksumInstallPackages
        ERROR_VARIABLE error
    )

    string(REGEX REPLACE " .*" "" checksumInstallPackages "${checksumInstallPackages}")

    execute_process(
        COMMAND ${SHA256SUM} -a 256 ${CMAKE_SOURCE_DIR}/systemInstallation/common
        RESULT_VARIABLE result
        OUTPUT_VARIABLE checksumCommon
        ERROR_VARIABLE error
    )

    string(REGEX REPLACE " .*" "" checksumCommon "${checksumCommon}")

    set(CHECKSUM_SYSTEM_INSTALLATION ${checksumSystemInstallation})
    set(CHECKSUM_INSTALL_PACKAGES ${checksumInstallPackages})
    set(CHECKSUM_COMMON ${checksumCommon})

    target_compile_definitions(delphinos-installer-elevated PUBLIC
        SYSTEM_INSTALLATION_CHECKSUM="${CHECKSUM_SYSTEM_INSTALLATION}"
        INSTALL_PACKAGES_CHECKSUM="${CHECKSUM_INSTALL_PACKAGES}"
        COMMON_CHECKSUM="${CHECKSUM_COMMON}"
    )
    
    # Print the checksums to verify
    message(STATUS "Checksum for systemInstallation.sh: ${checksumSystemInstallation}")
    message(STATUS "Checksum for installPackages.sh: ${checksumInstallPackages}")
    message(STATUS "Checksum for common: ${checksumCommon}")
    
    else()
    message(FATAL_ERROR "shasum não encontrado no seu sistema!")
endif()

# Install the executable (optional)
install(TARGETS delphinos-installer DESTINATION build)