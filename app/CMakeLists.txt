# Create the EngineTest executable
add_executable(EngineTest main.cpp )

# Include directories for the EngineTest executable
target_include_directories(EngineTest PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/external
)

# Link libraries to the EngineTest executable
target_link_libraries(EngineTest Engine)

# Copy shader files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)