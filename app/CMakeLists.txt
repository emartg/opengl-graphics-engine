## CMakeLists.txt for the App executable, that uses the Core and Platform libraries
## to create a simple OpenGL application

# Collect source files from the app directory
set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SOURCES_DIR}/*.cpp)

# Output each file in the list of sources for the App executable (debugging purposes)
message(STATUS "[APP] Source files:")
foreach(SOURCE ${SOURCES})
	message(STATUS "[APP] Source file: ${SOURCE}")
endforeach()

# Create the App executable
add_executable(App ${SOURCES})

# Include directories for the App executable
target_include_directories(App PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries to the App executable
# (the Platform library provides the Core library, GLFW, and ImGui)
target_link_libraries(App PRIVATE Engine::Platform)
target_compile_features(App PRIVATE cxx_std_20)

# Stage the bundled runtime dependencies (DLLs) beside the executable for local runs (Windows only).
# Resources are not copied: the Core locates them at runtime (beside the executable, in the
# installation layout, in the working directory, or in the Engine's source tree for development builds).
engine_stage_runtime_dependencies(App)

# Install the bundled runtime dependencies beside the installed executable too
if(WIN32 AND ENGINE_USE_BUNDLED_DEPS)
	file(GLOB ENGINE_RUNTIME_DLLS CONFIGURE_DEPENDS
		"${ENGINE_ROOT_DIR}/external/dlls/*.dll"
	)
	install(FILES ${ENGINE_RUNTIME_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# Smoke test: render a fixed number of frames of the example scene and exit.
# It fails if the App exits with an error code or prints an error message, and it requires a display
# with OpenGL 4.5 support (e.g., a virtual X server with Mesa's llvmpipe software renderer on Linux)
if(ENGINE_BUILD_TESTS)
	add_test(NAME App_Smoke_Test COMMAND App --frames 120)
	set_tests_properties(App_Smoke_Test PROPERTIES
		FAIL_REGULAR_EXPRESSION "\\[ERROR"
		TIMEOUT 300
		LABELS smoke
	)
endif()

install(TARGETS App RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${ENGINE_ROOT_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/EngineProject/resources)
