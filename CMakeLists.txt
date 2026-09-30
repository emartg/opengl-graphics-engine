## Top-level CMakeLists.txt for the Engine project.
## It can be configured on its own (to build the Core library and the sample App)
## or consumed by another CMake project through the add_subdirectory() command
## (e.g., as a Git submodule), in which case only the Core library is built by default

cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

# Project name, version and languages
project(EngineProject VERSION 0.9.0 LANGUAGES C CXX)

# Set C++ standard (only when the Engine is the top-level project,
# so that other consumers can keep their own C++ standard)
if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Options (the sample application is built by default only when the Engine is the top-level project)
option(ENGINE_BUILD_APP "Build the sample application" ${PROJECT_IS_TOP_LEVEL})
option(ENGINE_BUILD_PLATFORM "Build the Platform library (GLFW window, input, and ImGui)" ON)
option(ENGINE_BUILD_TESTS "Build engine tests" OFF)
option(ENGINE_USE_BUNDLED_DEPS "Allow the repository's dependency fallback" ON)

# Set default build type to Debug if not specified (single-configuration generators only, like Makefile or Ninja)
if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

include(GNUInstallDirs)

# Gather every binary in the same directories (only when Engine is the top-level project
# so that consumers decide where their own binaries are placed)
if (PROJECT_IS_TOP_LEVEL)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# Set root directory of the Engine sources, usable by consumers (e.g., to locate the Engine's resources)
set (ENGINE_ROOT_DIR "${PROJECT_SOURCE_DIR}" CACHE INTERNAL "Root directory of the Engine sources")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
include("${PROJECT_SOURCE_DIR}/cmake/EngineDependencies.cmake")

# The sample application requires the Platform library
if(ENGINE_BUILD_APP AND NOT ENGINE_BUILD_PLATFORM)
	message(FATAL_ERROR "ENGINE_BUILD_APP requires ENGINE_BUILD_PLATFORM to be ON")
endif()

# Add subdirectories
add_subdirectory(core)
if(ENGINE_BUILD_PLATFORM)
	add_subdirectory(platform)
endif()
if(ENGINE_BUILD_APP)
	add_subdirectory(app)
endif()
