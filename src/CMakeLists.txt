# Collect header, source, and shader files
set(HEADERS_SRC ${CMAKE_SOURCE_DIR}/includes/engine)
file(GLOB_RECURSE HEADERS ${HEADERS_SRC}/*.h*)

set(SOURCES_SRC ${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE SOURCES ${SOURCES_SRC}/*.cpp)

set(SHADERS_SRC ${CMAKE_SOURCE_DIR}/shaders)
file(GLOB_RECURSE SHADERS ${SHADERS_SRC}/*.glsl)

# Include current directory
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Create the Engine static library
add_library(Engine STATIC ${HEADERS} ${SOURCES} ${SHADERS})

# Include directories for the Engine library
target_include_directories(Engine PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
)

# Link libraries to the Engine library
target_link_libraries(Engine ${LIBS})
