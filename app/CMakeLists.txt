## CMakeLists.txt for the App executable, that uses the Core library 
## to create a simple OpenGL application

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(GLFW3 REQUIRED)
message(STATUS "[APP] Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# Collect source files from the app directory
set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/app)
file(GLOB_RECURSE SOURCES ${SOURCES_DIR}/*.cpp)

#Collect ImGui backend source files
set (IMGUI_BACKEND_DIR ${CMAKE_SOURCE_DIR}/external/include/ImGui/backends)
file(GLOB IMGUI_BACKEND_SOURCES ${IMGUI_BACKEND_DIR}/*.cpp)

# Collect ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/include/ImGui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

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
add_executable(App ${SOURCES} ${IMGUI_BACKEND_SOURCES} ${IMGUI_SOURCES})

# Include directories for the App executable
target_include_directories(App PUBLIC
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/external/include/ImGui
)

# List of libraries to link against, including the ImGui library
set(LIBS
	${GLFW3_LIBRARY}
	${ASSIMP_LIBRARY}
)

# Link libraries to the App executable
target_link_libraries(App Core ${LIBS})

# Copy shader files to the runtime directory
message(STATUS "[APP] Copying shader files to the runtime directory...")
file(INSTALL ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)

# Copy asset files to the runtime directory
message(STATUS "[APP] Copying asset files to the runtime directory...")
file(INSTALL ${CMAKE_SOURCE_DIR}/assets/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)