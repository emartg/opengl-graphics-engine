## CMakeLists.txt for the Core library, that provides the basic functionality
## for creating an OpenGL application

# Collect header, source, and shader files
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${HEADERS_DIR}/*.h*)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SOURCES_DIR}/*.cpp)

set(SHADERS_DIR ${ENGINE_ROOT_DIR}/resources/shaders)
file(GLOB SHADERS CONFIGURE_DEPENDS ${SHADERS_DIR}/*.glsl)

# Output each file in the list of headers, sources, and shaders for the Core library (debugging purposes)
foreach(FILE_TYPE IN ITEMS HEADERS SOURCES SHADERS)
    message(STATUS "[CORE] ${FILE_TYPE} files:")
    foreach(FILE ${${FILE_TYPE}})
        message(STATUS "[CORE] ${FILE_TYPE} file: ${FILE}")
    endforeach()
endforeach()

# Create the Core static library (headers and sources are listed so the IDEs display them),
# and a namespaced alias for consumers (e.g., Engine::Core)
add_library(Core STATIC ${SOURCES} ${HEADERS} ${SHADERS})
add_library(Engine::Core ALIAS Core)

# Include directories for the Core library:
# - the Engine root directory, so that Core headers can be included as "core/<path>/<header>.h"
# - the headers of the dependencies included in the source tree (GLAD, KHR, and stb_image)
target_include_directories(Core PUBLIC
	$<BUILD_INTERFACE:${ENGINE_ROOT_DIR}>
	$<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/external/include>
)

# Link libraries to the Core library
target_link_libraries(Core PUBLIC
	OpenGL::GL
	glm::glm
	assimp::assimp
	engine_glad
	engine_stb_image
)
target_compile_features(Core PUBLIC cxx_std_20)

# Locations searched for the resources directory at runtime (see Core::resolve_resources_dir):
# - ENGINE_INSTALL_RESOURCES_DIR: relative to the installation prefix (matches the install rule of the App)
# - ENGINE_SOURCE_RESOURCES_DIR: the Engine's source tree, used by development builds (resources are not
#   copied into the build tree, so changes to shaders and other resources are used without rebuilding)
target_compile_definitions(Core PRIVATE
	ENGINE_INSTALL_RESOURCES_DIR="${CMAKE_INSTALL_DATADIR}/EngineProject/resources"
	ENGINE_SOURCE_RESOURCES_DIR="${ENGINE_ROOT_DIR}/resources"
)

install(TARGETS Core
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Organize files into source groups for Visual Studio
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})
source_group("Shader Files" FILES ${SHADERS})
