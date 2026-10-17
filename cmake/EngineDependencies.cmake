# Resolve third-party dependencies and expose stable targets to the project.
#
# GLFW, GLM, Assimp, and nlohmann/json are located through the CMake packages they install (find_package in CONFIG mode),
# which are provided by vcpkg on Windows (see vcpkg.json) and by the system packages on Linux.
# GLAD, stb_image, ImGui, and ImGuiFileDialog are part of the source tree (external/include and core/).

find_package(OpenGL REQUIRED)

# Resolves a required dependency installed as a CMake package, with a common error message
# that explains how to provide it on each platform
macro(engine_find_package package)
    find_package(${package} CONFIG QUIET)
    if(NOT ${package}_FOUND)
        message(FATAL_ERROR "${package} was not found.\n"
            "On Windows, use a preset (or the vcpkg toolchain: -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake), "
            "so that vcpkg installs the dependencies of vcpkg.json. "
            "On Linux, install the development packages (e.g., libglfw3-dev, libassimp-dev, libglm-dev, and nlohmann-json3-dev).\n"
            "If the build directory was configured before without vcpkg, reconfigure it from scratch (cmake --fresh).")
    endif()
endmacro()

engine_find_package(glm)           # target: glm::glm
engine_find_package(glfw3)         # target: glfw
engine_find_package(assimp)        # target: assimp::assimp
engine_find_package(nlohmann_json) # target: nlohmann_json::nlohmann_json

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
    target_link_libraries(engine_imgui PUBLIC glfw OpenGL::GL)
    target_compile_features(engine_imgui PUBLIC cxx_std_17)
endif()

# Copies the runtime dependencies (DLLs) built by vcpkg beside the executable of the given target, so that it can be
# run from the build tree. It is only needed with MinGW: with MSVC, vcpkg already copies the DLLs used by each
# executable, and on Linux the shared libraries are found by the system. It is a function (functions are global in
# CMake), so consumers of the Engine can use it for their executables.
# The copy is done by one custom target for each output directory, which the executables of that directory depend on, so
# that several executables that share a folder (e.g., the App and the unit tests) never copy the same files at the
# same time in a parallel build (a copy that finds a file open by another one fails with "permission denied")
function(engine_stage_runtime_dependencies target)
    if(MINGW AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        # every DLL installed by vcpkg (including the dependencies of Assimp, e.g., zlib), for each configuration
        set(_engine_vcpkg_dir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
        file(GLOB _engine_release_dlls "${_engine_vcpkg_dir}/bin/*.dll")
        file(GLOB _engine_debug_dlls "${_engine_vcpkg_dir}/debug/bin/*.dll")
        if(_engine_release_dlls AND _engine_debug_dlls)
            # the output directory of the target, as it is when the function is called (the same rules as CMake)
            get_target_property(_engine_output_dir ${target} RUNTIME_OUTPUT_DIRECTORY)
            if(NOT _engine_output_dir)
                set(_engine_output_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            endif()
            if(NOT _engine_output_dir)
                set(_engine_output_dir "${CMAKE_CURRENT_BINARY_DIR}")
            endif()

            # the custom target of that directory, created by the first executable that asks for it
            string(MD5 _engine_output_dir_hash "${_engine_output_dir}")
            string(SUBSTRING "${_engine_output_dir_hash}" 0 8 _engine_output_dir_hash)
            get_property(_engine_stager GLOBAL PROPERTY ENGINE_RUNTIME_STAGER_${_engine_output_dir_hash})
            if(NOT _engine_stager)
                set(_engine_stager engine_runtime_dependencies_${_engine_output_dir_hash})
                add_custom_target(${_engine_stager}
                    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<IF:$<CONFIG:Debug>,${_engine_debug_dlls},${_engine_release_dlls}>"
                        "$<TARGET_FILE_DIR:${target}>"
                    COMMAND_EXPAND_LISTS
                    COMMENT "Copying the runtime dependencies installed by vcpkg beside ${target}"
                )
                set_property(GLOBAL PROPERTY ENGINE_RUNTIME_STAGER_${_engine_output_dir_hash} ${_engine_stager})
            endif()
            add_dependencies(${target} ${_engine_stager})
        endif()
    endif()
endfunction()
