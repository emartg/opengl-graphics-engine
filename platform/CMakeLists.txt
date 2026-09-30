## CMakeLists.txt for the Platform library, that implements the window, OpenGL context,
## input, and ImGui integration of the Core renderer with GLFW.
## It is used by the sample App and by any other application that needs a window
## (e.g., the X-ray simulator), while the Core library remains independent of GLFW and ImGui

# Collect header and source files
file(GLOB_RECURSE PLATFORM_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE PLATFORM_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Create the Platform static library (headers are listed so that IDEs display them),
# and a namespaced alias for consumers (e.g., Engine::Platform)
add_library(Platform STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})
add_library(Engine::Platform ALIAS Platform)

# Link libraries to the Platform library (the Core library provides the Engine root include directory,
# so that Platform headers can be included as "platform/<path>/<header>.h")
target_link_libraries(Platform PUBLIC
	Engine::Core
	engine_imgui
	${ENGINE_GLFW_TARGET}
)
target_compile_features(Platform PUBLIC cxx_std_20)

install(TARGETS Platform
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Organize files into source groups for Visual Studio
source_group("Header Files" FILES ${PLATFORM_HEADERS})
source_group("Source Files" FILES ${PLATFORM_SOURCES})
