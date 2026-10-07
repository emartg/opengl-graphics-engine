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

# Stage the runtime dependencies (DLLs) beside the executable for local runs (MinGW only, see the function).
# Resources are not copied: the Core locates them at runtime (beside the executable, in the
# installation layout, in the working directory, or in the Engine's source tree for development builds).
engine_stage_runtime_dependencies(App)

# Install the runtime dependencies (DLLs) staged beside the executable, by vcpkg (MSVC) or by
# engine_stage_runtime_dependencies (MinGW), beside the installed executable too (Windows only)
if(WIN32)
	install(DIRECTORY "$<TARGET_FILE_DIR:App>/"
		DESTINATION ${CMAKE_INSTALL_BINDIR}
		COMPONENT App
		FILES_MATCHING PATTERN "*.dll"
	)
endif()

# Install the MSVC runtime libraries beside the executable too, so that the installed App runs on
# computers without the Visual C++ Redistributable (e.g., the package attached to the releases)
if(MSVC)
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT App)
	include(InstallRequiredSystemLibraries)
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

# Install the App and the resources in the App component, so that they can be installed without the
# Core and Platform libraries (e.g., cmake --install <build dir> --component App)
install(TARGETS App RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT App)
install(DIRECTORY ${ENGINE_ROOT_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/EngineProject/resources COMPONENT App)
