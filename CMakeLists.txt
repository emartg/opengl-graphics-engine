## Top-level CMakeLists.txt for the Engine project, that combines the Core library and the App executable

cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)

# Project name and version
project(EngineProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Add custom CMake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Set output directory for archives, libraries, and runtime on different platforms
if(WIN32)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	set(BUILD_SHARED_LIBS OFF)
endif()

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(GLM REQUIRED)
message(STATUS "[Engine] GLM included at ${GLM_INCLUDE_DIR}")

# Add subdirectories
add_subdirectory(core)
add_subdirectory(app)