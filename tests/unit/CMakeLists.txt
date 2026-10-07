## CMakeLists.txt for the unit tests of the Engine, written with GoogleTest.
## They test the Core's classes that do not require an OpenGL context or a window,
## so they run on every platform and CI runner (unlike the App smoke test)

## Test suites and test names follow GoogleTest's naming convention (PascalCase, without underscores),
## since GoogleTest builds a class name from them by joining them with an underscore

# GoogleTest: use an installed package if available (e.g., a system package), and otherwise
# download a pinned release (the hash guarantees that the downloaded archive is the expected one)
find_package(GTest 1.14 CONFIG QUIET)
if(NOT TARGET GTest::gtest_main)
	include(FetchContent)
	FetchContent_Declare(googletest
		URL https://github.com/google/googletest/archive/v1.18.0.tar.gz
		URL_HASH SHA512=ba0f5769ccf34acf1bc72d1f7e9ffb8202176d02b64f6f3d9047accfc0cf9026ff5a653d24935e2705fff8709566676452616c93ca0ca6277f1e21d79b58a10a
		DOWNLOAD_EXTRACT_TIMESTAMP ON
	)
	# use the dynamic C runtime with MSVC (as the Engine does), and build only GoogleTest (not GoogleMock),
	# without adding its install rules to the Engine's
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
endif()
message(STATUS "[TESTS] GoogleTest: ${GTest_DIR}${googletest_SOURCE_DIR}")

# Collect the test source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Create the unit test executable, linked against the Core library and GoogleTest (with its main function)
add_executable(Engine_Unit_Tests ${SOURCES})
target_link_libraries(Engine_Unit_Tests PRIVATE Engine::Core GTest::gtest_main)
target_compile_features(Engine_Unit_Tests PRIVATE cxx_std_20)

# Name of the executable file, used by the tests of File_System_Utils::get_executable_directory()
target_compile_definitions(Engine_Unit_Tests PRIVATE
	ENGINE_UNIT_TESTS_EXECUTABLE_NAME="$<TARGET_FILE_NAME:Engine_Unit_Tests>"
)

# Copy the runtime dependencies (DLLs) beside the executable (MinGW only, see the function)
engine_stage_runtime_dependencies(Engine_Unit_Tests)

# Register every GoogleTest test as a CTest test with the "unit" label. The tests are listed when
# CTest runs (not after the build), so the runtime dependencies are already in place
include(GoogleTest)
gtest_discover_tests(Engine_Unit_Tests
	DISCOVERY_MODE PRE_TEST
	PROPERTIES LABELS unit
)