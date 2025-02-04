## CMakeLists.txt for the Core library, that provides the basic functionality
## for creating an OpenGL application

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(OpenGL REQUIRED)
message(STATUS "[CORE] OpenGL included at ${OPENGL_INCLUDE_DIR}")

find_package(GLM REQUIRED)
message(STATUS "[CORE] GLM included at ${GLM_INCLUDE_DIR}")

find_package(Assimp REQUIRED)
message(STATUS "[CORE] Found Assimp in ${ASSIMP_INCLUDE_DIR}")

# List of libraries to link against
set(LIBS
	${OPENGL_gl_LIBRARY}
	${ASSIMP_LIBRARY}
)

# Create static libraries for GLAD and STB_IMAGE
add_library(GLAD STATIC ${CMAKE_SOURCE_DIR}/core/glad.c)
add_library(STB_IMAGE STATIC ${CMAKE_SOURCE_DIR}/core/stb_image.cpp)

# Include directories for the GLAD library and STB_IMAGE library
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/external/include)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/external/include)

# Add GLAD and STB_IMAGE to the list of libraries to link against
list(APPEND LIBS GLAD)
list(APPEND LIBS STB_IMAGE)

# Collect header, source, and shader files
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/core)
file(GLOB_RECURSE HEADERS ${HEADERS_DIR}/*.h*)

set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/core)
file(GLOB_RECURSE SOURCES ${SOURCES_DIR}/*.cpp)

set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
file(GLOB SHADERS ${SHADERS_DIR}/*.glsl)

# Create the Core static library
add_library(Core STATIC ${HEADERS} ${SOURCES})

# Include directories for the Core library
target_include_directories(Core PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/external
)

# Link libraries to the Core library
target_link_libraries(Core ${LIBS})

# Organize files into source groups for Visual Studio
source_group("Header Files" FILES ${HEADERS_DIR})
source_group("Source Files" FILES ${SOURCES_DIR})
source_group("Shader Files" FILES ${SHADERS_DIR})