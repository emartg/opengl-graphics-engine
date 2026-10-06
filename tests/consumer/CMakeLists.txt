## CMakeLists.txt for the consumer test, a standalone project that uses the Engine as an external
## dependency through add_subdirectory(), as the X-ray simulator does with the Engine as a Git submodule.
## It verifies that the Engine can be consumed, that the sample App is not built for consumers,
## and that an application can create a window and its own GUI with the Platform library

cmake_minimum_required(VERSION 3.21)

project(Engine_Consumer_Test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the Engine from the repository root (a binary directory is required, since it is not a subdirectory)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. Engine)

# The sample App must not be built when the Engine is consumed by another project
if(TARGET App)
	message(FATAL_ERROR "The Engine's sample App must not be built for consumers")
endif()

# Create the consumer executable, linked only against the Platform library (which provides the Core library)
add_executable(Consumer_Test main.cpp)
target_link_libraries(Consumer_Test PRIVATE Engine::Platform)

# Copy the Engine's bundled runtime dependencies (DLLs) beside the executable (Windows only)
engine_stage_runtime_dependencies(Consumer_Test)

# Run test: create a window, render a few frames with a custom GUI layer, and exit.
# It fails if the executable exits with an error code or prints an error message
enable_testing()
add_test(NAME Consumer_Test COMMAND Consumer_Test)
set_tests_properties(Consumer_Test PROPERTIES
	FAIL_REGULAR_EXPRESSION "\\[ERROR"
	TIMEOUT 300
)
