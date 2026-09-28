## CMakeLists.txt for the App executable, that uses the Core library 
## to create a simple OpenGL application

# Collect source files from the app directory
set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${SOURCES_DIR}/*.cpp)

# Collect ImGui backend source files
set(IMGUI_BACKEND_DIR ${ENGINE_ROOT_DIR}/external/include/ImGui/backends)
file(GLOB IMGUI_BACKEND_SOURCES CONFIGURE_DEPENDS ${IMGUI_BACKEND_DIR}/*.cpp)

# Collect ImGui source files
set(IMGUI_DIR ${ENGINE_ROOT_DIR}/external/include/ImGui)
file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS ${IMGUI_DIR}/*.cpp)

# Collect ImGuiFileDialog source files
set(IMGUI_FILE_DIALOG_DIR ${ENGINE_ROOT_DIR}/external/include/ImGuiFileDialog)
file(GLOB IMGUI_FILE_DIALOG_SOURCES CONFIGURE_DEPENDS ${IMGUI_FILE_DIALOG_DIR}/*.cpp)

# Output each file in the list of sources for the App executable (debugging purposes)
message(STATUS "[APP] Source files:")
foreach(SOURCE ${SOURCES})
	message(STATUS "[APP] Source file: ${SOURCE}")
endforeach()

# Output each file in the list of ImGui backend sources (debugging purposes)
message(STATUS "[APP] ImGui backend files:")
foreach(SOURCE ${IMGUI_BACKEND_SOURCES})
	message(STATUS "[APP] ImGui backend file: ${SOURCE}")
endforeach()

# Output each file in the list of ImGui sources (debugging purposes)
message(STATUS "[APP] ImGui files:")
foreach(SOURCE ${IMGUI_SOURCES})
	message(STATUS "[APP] ImGui file: ${SOURCE}")
endforeach()

# Create the App executable
add_executable(App ${SOURCES} ${IMGUI_BACKEND_SOURCES} ${IMGUI_SOURCES} ${IMGUI_FILE_DIALOG_SOURCES})

# Include directories for the App executable
target_include_directories(App PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${ENGINE_ROOT_DIR}/external/include/ImGui
	${ENGINE_ROOT_DIR}/external/include/ImGuiFileDialog
)

# Link libraries to the App executable
target_link_libraries(App PRIVATE Core ${ENGINE_GLFW_TARGET})
target_compile_features(App PRIVATE cxx_std_20)

# Stage runtime dependencies and resources beside the executable for local runs.
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
endif()

add_custom_command(TARGET App POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${ENGINE_ROOT_DIR}/resources"
		"$<TARGET_FILE_DIR:App>/resources"
)

install(TARGETS App RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${ENGINE_ROOT_DIR}/resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/EngineProject/resources)