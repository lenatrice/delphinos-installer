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

set(CHECKSUM_SYSTEM_INSTALLATION "UNKNOWN")
set(CHECKSUM_INSTALL_PACKAGES "UNKNOWN")
set(CHECKSUM_COMMON "UNKNOWN")

macro(CALCULATE_CHECKSUM FILE_VAR FILE_PATH)
    if(EXISTS "${FILE_PATH}")
        execute_process(
            COMMAND ${SHA256SUM} -a 256 "${FILE_PATH}"
            OUTPUT_VARIABLE output
            ERROR_VARIABLE error
            RESULT_VARIABLE result
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(result EQUAL 0)
            string(REGEX REPLACE " .*" "" ${FILE_VAR} "${output}")
        else()
            message(WARNING "Failed to calculate checksum for ${FILE_PATH}: ${error}")
        endif()
    else()
        message(WARNING "File not found: ${FILE_PATH}")
    endif()
endmacro()

CALCULATE_CHECKSUM(CHECKSUM_SYSTEM_INSTALLATION "${CMAKE_SOURCE_DIR}/systemInstallation/systemInstallation.sh")
CALCULATE_CHECKSUM(CHECKSUM_INSTALL_PACKAGES "${CMAKE_SOURCE_DIR}/systemInstallation/installPackages.sh")
CALCULATE_CHECKSUM(CHECKSUM_COMMON "${CMAKE_SOURCE_DIR}/systemInstallation/common")

target_compile_definitions(delphinos-installer-elevated PUBLIC
    SYSTEM_INSTALLATION_CHECKSUM="${CHECKSUM_SYSTEM_INSTALLATION}"
    INSTALL_PACKAGES_CHECKSUM="${CHECKSUM_INSTALL_PACKAGES}"
    COMMON_CHECKSUM="${CHECKSUM_COMMON}"
)

message(STATUS "systemInstallation.sh script checksum: ${CHECKSUM_SYSTEM_INSTALLATION}")
message(STATUS "installPackages.sh script checksum: ${CHECKSUM_INSTALL_PACKAGES}")
message(STATUS "common script checksum: ${CHECKSUM_COMMON}")

install(TARGETS delphinos-installer-elevated DESTINATION build)