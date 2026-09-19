## Top-level CMakeLists.txt for the Engine project.

cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)

# Project name, version and languages
project(EngineProject VERSION 0.5.2 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENGINE_BUILD_APP "Build the sample application" ON)
option(ENGINE_BUILD_TESTS "Build engine tests" OFF)
option(ENGINE_USE_BUNDLED_DEPS "Allow the repository's dependency fallback" ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

include(GNUInstallDirs)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
include("${CMAKE_SOURCE_DIR}/cmake/EngineDependencies.cmake")

# Add subdirectories
add_subdirectory(core)
if(ENGINE_BUILD_APP)
    add_subdirectory(app)
endif()