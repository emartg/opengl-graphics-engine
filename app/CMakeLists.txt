## CMakeLists.txt for the App executable, that uses the Core library 
## to create a simple OpenGL application

# Create the App executable
add_executable(App main.cpp )

# Include directories for the App executable
target_include_directories(App PUBLIC
	${OPENGL_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/external
)

# Link libraries to the App executable
target_link_libraries(App Core)

# Copy shader files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)

# Copy resource files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/)