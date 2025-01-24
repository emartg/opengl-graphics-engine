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

# Set output directory for runtime binaries on Windows
if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL included at ${OPENGL_INCLUDE_DIR}")

find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

find_package(Assimp REQUIRED)
message(STATUS "Found Assimp in ${ASSIMP_INCLUDE_DIR}")

# List of libraries to link against
set(LIBS
	${OPENGL_gl_LIBRARY}
	${GLFW3_LIBRARY}
	${ASSIMP_LIBRARY}
)

# Create static libraries for GLAD and STB_IMAGE
add_library(GLAD STATIC ${CMAKE_SOURCE_DIR}/engine/glad.c)
add_library(STB_IMAGE STATIC ${CMAKE_SOURCE_DIR}/engine/stb_image.cpp)

# Include directories for the GLAD library and STB_IMAGE library
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/external/include)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/external/include)

# Add GLAD and STB_IMAGE to the list of libraries to link against
list(APPEND LIBS GLAD)
list(APPEND LIBS STB_IMAGE)

# Collect header, source, and shader files
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/engine)
file(GLOB_RECURSE HEADERS ${HEADERS_DIR}/*.h*)

set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/engine)
file(GLOB_RECURSE SOURCES ${SOURCES_DIR}/*.cpp)

set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
file(GLOB_RECURSE SHADERS ${SHADERS_DIR}/*.glsl)

# Create the Engine static library
add_library(Engine STATIC ${HEADERS} ${SOURCES} ${SHADERS})

# Include directories for the Engine library
target_include_directories(Engine PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/external
)

# Link libraries to the Engine library
target_link_libraries(Engine ${LIBS})

# Create the EngineTest executable
add_executable(EngineTest ${CMAKE_SOURCE_DIR}/engine/test/test.cpp)

# Include directories for the EngineTest executable
target_include_directories(EngineTest PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/external
)

# Link libraries to the EngineTest executable
target_link_libraries(EngineTest Engine ${LIBS})

# Copy shader files to the runtime directory
file(INSTALL shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)

# Organize files into source groups for Visual Studio
source_group("Header Files" FILES ${HEADERS_DIR})
source_group("Source Files" FILES ${SOURCES_DIR})
source_group("Shader Files" FILES ${SHADERS_DIR})