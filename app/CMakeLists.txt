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

# Stage the bundled runtime dependencies (DLLs) beside the executable for local runs.
# Resources are not copied: the Core locates them at runtime (beside the executable, in the
# installation layout, in the working directory, or in the Engine's source tree for development builds).
if(WIN32 AND ENGINE_USE_BUNDLED_DEPS)
	file(GLOB ENGINE_RUNTIME_DLLS CONFIGURE_DEPENDS
		"${ENGINE_ROOT_DIR}/external/dlls/*.dll"
	)

	foreach(RUNTIME_DLL IN LISTS ENGINE_RUNTIME_DLLS)
		add_custom_command(TARGET App POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${RUNTIME_DLL}"
				"$<TARGET_FILE_DIR:App>"
		)
	endforeach()

	# install the bundled DLLs beside the installed executable too
	install(FILES ${ENGINE_RUNTIME_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(TARGETS App RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${ENGINE_ROOT_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/EngineProject/resources)