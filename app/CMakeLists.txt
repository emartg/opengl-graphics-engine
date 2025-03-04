## CMakeLists.txt for the App executable, that uses the Core library 
## to create a simple OpenGL application

# Add library directories
link_directories(${CMAKE_BINARY_DIR}/lib)

# Find and include required packages
find_package(GLFW3 REQUIRED)
message(STATUS "[APP] Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# Collect ImGui header and source files
set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/ImGui)
file(GLOB IMGUI_HEADERS ${IMGUI_INCLUDE_DIR}/*.h)
file(GLOB IMGUI_SOURCES ${IMGUI_INCLUDE_DIR}/*.cpp)

# Create the ImGui library
add_library(ImGui STATIC ${IMGUI_HEADERS} ${IMGUI_SOURCES})

# Link ImGui against the required libraries
target_link_libraries(ImGui ${GLFW3_LIBRARY})

# Include directories for the ImGui library
target_include_directories(ImGui PUBLIC
	${GLFW3_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}/backends
)

# Collect header and source files
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/app)
file(GLOB_RECURSE HEADERS ${HEADERS_DIR}/*.h*)

set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/app)
file(GLOB_RECURSE SOURCES ${SOURCES_DIR}/*.cpp)

# Create the App executable
add_executable(App ${HEADERS} ${SOURCES})

# Include directories for the App executable
target_include_directories(App PUBLIC
	${GLM_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIR}
	${IMGUI_INCLUDE_DIR}
)

# List of libraries to link against, including the ImGui library
set(LIBS
	${GLFW3_LIBRARY}
	${ASSIMP_LIBRARY}
	ImGui
)

# Link libraries to the App executable
target_link_libraries(App Core ${LIBS})

# Copy shader files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)

# Copy shader files to the runtime directory
file(INSTALL ${CMAKE_SOURCE_DIR}/assets/textures/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/textures/)