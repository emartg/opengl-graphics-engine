# Resolve third-party dependencies and expose stable targets to the project.

find_package(OpenGL REQUIRED)

if(TARGET OpenGL::GL)
    set(_engine_opengl_target OpenGL::GL)
elseif(TARGET OpenGL::OpenGL)
    set(_engine_opengl_target OpenGL::OpenGL)
else()
    message(FATAL_ERROR "OpenGL was found, but no usable imported target was provided")
endif()

find_package(glm CONFIG QUIET)
if(NOT TARGET glm::glm)
    find_package(GLM QUIET)
endif()
if(NOT TARGET glm::glm)
    if(EXISTS "${PROJECT_SOURCE_DIR}/external/include/glm/glm.hpp")
        add_library(engine_glm INTERFACE)
        target_include_directories(engine_glm INTERFACE "${PROJECT_SOURCE_DIR}/external/include")
        add_library(glm::glm ALIAS engine_glm)
    else()
        message(FATAL_ERROR "GLM was not found. Install GLM or ensure external/include/glm is available.")
    endif()
endif()

find_package(glfw3 CONFIG QUIET)
if(NOT TARGET glfw AND NOT TARGET glfw3 AND NOT TARGET glfw::glfw AND NOT TARGET glfw3::glfw3)
    if(MINGW)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(GLFW3 QUIET IMPORTED_TARGET glfw3)
        endif()
    endif()

    if(NOT TARGET PkgConfig::GLFW3)
        find_package(GLFW3 QUIET)
        if(GLFW3_FOUND)
            add_library(engine_glfw UNKNOWN IMPORTED)
            set_target_properties(engine_glfw PROPERTIES
                IMPORTED_LOCATION "${GLFW3_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIR}"
            )
            add_library(glfw ALIAS engine_glfw)
        endif()
    endif()
endif()

# The bundled GLFW library is distributed without its PDB file, so MSVC Debug links report
# warning LNK4099 for every GLFW object (they are linked without debug information).
# Ignore it for the bundled library only, and for every target that links it
cmake_path(IS_PREFIX ENGINE_ROOT_DIR "${GLFW3_LIBRARY}" _engine_glfw_is_bundled)
if(MSVC AND TARGET engine_glfw AND _engine_glfw_is_bundled)
    set_property(TARGET engine_glfw APPEND PROPERTY INTERFACE_LINK_OPTIONS "/IGNORE:4099")
endif()

if(TARGET glfw)
    set(_engine_glfw_target glfw)
elseif(TARGET glfw3)
    set(_engine_glfw_target glfw3)
elseif(TARGET glfw3::glfw3)
    set(_engine_glfw_target glfw3::glfw3)
elseif(TARGET glfw::glfw)
    set(_engine_glfw_target glfw::glfw)
elseif(TARGET PkgConfig::GLFW3)
    set(_engine_glfw_target PkgConfig::GLFW3)
else()
    message(FATAL_ERROR "GLFW was not found.\n"
        "Please install GLFW (e.g. vcpkg: 'vcpkg install glfw3' or your system package manager), or provide GLFW3_LIBRARY and GLFW3_INCLUDE_DIR variables.\n"
        "CMake search paths: CMAKE_PREFIX_PATH='${CMAKE_PREFIX_PATH}'\n"
        "If using vcpkg, ensure CMake toolchain is set via -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake.")
endif()

find_package(assimp CONFIG QUIET)
if(NOT TARGET assimp::assimp AND NOT TARGET assimp)
    if(MINGW)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(ASSIMP QUIET IMPORTED_TARGET assimp)
        endif()
    endif()

    if(NOT TARGET PkgConfig::ASSIMP)
        find_package(Assimp QUIET)
        if(ASSIMP_FOUND)
            add_library(engine_assimp UNKNOWN IMPORTED)
            set_target_properties(engine_assimp PROPERTIES
                IMPORTED_LOCATION "${ASSIMP_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}"
            )
            add_library(assimp::assimp ALIAS engine_assimp)
        endif()
    endif()
endif()

if(TARGET assimp::assimp)
    set(_engine_assimp_target assimp::assimp)
elseif(TARGET assimp)
    set(_engine_assimp_target assimp)
elseif(TARGET PkgConfig::ASSIMP)
    set(_engine_assimp_target PkgConfig::ASSIMP)
else()
    message(FATAL_ERROR "Assimp was not found. Install Assimp or configure ASSIMP_LIBRARY and ASSIMP_INCLUDE_DIR.")
endif()

add_library(engine_glad STATIC "${PROJECT_SOURCE_DIR}/core/glad.c")
target_include_directories(engine_glad PUBLIC "${PROJECT_SOURCE_DIR}/external/include")

add_library(engine_stb_image STATIC "${PROJECT_SOURCE_DIR}/core/stb_image.cpp")
target_include_directories(engine_stb_image PUBLIC "${PROJECT_SOURCE_DIR}/external/include")

# ImGui (with its GLFW and OpenGL 3+ backends) and ImGuiFileDialog, used by the Platform library
if(ENGINE_BUILD_PLATFORM)
    set(_engine_imgui_dir "${PROJECT_SOURCE_DIR}/external/include/ImGui")
    set(_engine_imgui_file_dialog_dir "${PROJECT_SOURCE_DIR}/external/include/ImGuiFileDialog")
    file(GLOB _engine_imgui_sources CONFIGURE_DEPENDS "${_engine_imgui_dir}/*.cpp")

    add_library(engine_imgui STATIC
        ${_engine_imgui_sources}
        "${_engine_imgui_dir}/backends/imgui_impl_glfw.cpp"
        "${_engine_imgui_dir}/backends/imgui_impl_opengl3.cpp"
        "${_engine_imgui_file_dialog_dir}/ImGuiFileDialog.cpp"
    )
    target_include_directories(engine_imgui PUBLIC
        "${_engine_imgui_dir}"
        "${_engine_imgui_file_dialog_dir}"
        "${PROJECT_SOURCE_DIR}/external/include"
    )
    target_link_libraries(engine_imgui PUBLIC ${_engine_glfw_target} ${_engine_opengl_target})
    target_compile_features(engine_imgui PUBLIC cxx_std_17)
endif()

set(ENGINE_GLFW_TARGET "${_engine_glfw_target}" CACHE INTERNAL "Resolved GLFW target")
set(ENGINE_OPENGL_TARGET "${_engine_opengl_target}" CACHE INTERNAL "Resolved OpenGL target")
set(ENGINE_ASSIMP_TARGET "${_engine_assimp_target}" CACHE INTERNAL "Resolved Assimp target")
