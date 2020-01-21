cmake_minimum_required(VERSION 3.6)

# output verbose info
set(CMAKE_VERBOSE_MAKEFILE off)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_BUGFIX 0)
set(VERSION_SUFFIX "")
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}${VERSION_SUFFIX}" CACHE STRING "" FORCE)

set(IC nRF52832)
# must be set as cache strings to ensure they are not overwritten in mesh SDK
set(SOFTDEVICE_TYPE "s132" CACHE STRING "SoftDevice type")
set(SOFTDEVICE_VERSION "7.0.1" CACHE STRING "SoftDevice version")

set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR} CACHE STRING "" FORCE)

# Windows users uncomment
# set(PATCH_EXECUTABLE "$ENV{ProgramFiles\(x86\)}/GnuWin32/bin/patch.exe")

include("${CMAKE_SOURCE_DIR}/nRF5-cmake-scripts/nRF5-cmake.cmake")

if(TARGET download)
    return()
endif()

# We enable the project() here for CMake to initialize variables s.a. "CMAKE_HOST_W32".
# Languages are enabled _after_ the toolchain has been setup.
project(ExampleProject
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUGFIX}
    LANGUAGES NONE)

message(STATUS "Configuring CMake for nRF5 SDK for ${PROJECT_NAME} ${VERSION_STRING}")
nRF5_setup()
add_subdirectory("src")
