## CMakeLists.txt for the Core library, that provides the basic functionality
## for creating an OpenGL application

# Collect header, source, and shader files
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/core)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${HEADERS_DIR}/*.h*)

set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/core)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SOURCES_DIR}/*.cpp)

set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/resources/shaders)
file(GLOB SHADERS CONFIGURE_DEPENDS ${SHADERS_DIR}/*.glsl)

# Output each file in the list of headers, sources, and shaders for the Core library (debugging purposes)
foreach(FILE_TYPE IN ITEMS HEADERS SOURCES SHADERS)
    message(STATUS "[CORE] ${FILE_TYPE} files:")
    foreach(FILE ${${FILE_TYPE}})
        message(STATUS "[CORE] ${FILE_TYPE} file: ${FILE}")
    endforeach()
endforeach()

# Create the Core static library
add_library(Core STATIC ${SOURCES})

# Include directories for the Core library
target_include_directories(Core PUBLIC
	${CMAKE_SOURCE_DIR}/external/include
)

# Link libraries to the Core library
target_link_libraries(Core PUBLIC
	${ENGINE_OPENGL_TARGET}
	glm::glm
	${ENGINE_ASSIMP_TARGET}
	engine_glad
	engine_stb_image
)
target_compile_features(Core PUBLIC cxx_std_20)

install(TARGETS Core
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Organize files into source groups for Visual Studio
source_group("Header Files" FILES ${HEADERS_DIR})
source_group("Source Files" FILES ${SOURCES_DIR})
source_group("Shader Files" FILES ${SHADERS_DIR})