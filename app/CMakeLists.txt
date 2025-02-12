## CMakeLists.txt for the App executable, that uses the Core library 
## to create a simple OpenGL application

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(GLFW3 REQUIRED)
message(STATUS "[APP] Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# List of libraries to link against
set(LIBS
	${GLFW3_LIBRARY}
	${ASSIMP_LIBRARY}
)

# Create the App executable
add_executable(App main.cpp)

# Include directories for the App executable
target_include_directories(App PUBLIC
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
)

# Link libraries to the App executable
target_link_libraries(App Core ${LIBS})

# Copy shader files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)