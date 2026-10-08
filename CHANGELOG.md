# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) (before v1.0.0, minor versions may include breaking changes). The detailed notes of each version are published in its [GitHub release](https://github.com/emartg/opengl-graphics-engine/releases).

## [Unreleased]

### Added

- `--scene NAME`, `--objects N`, and `--list-scenes` command-line options of the App, to choose the scene (and the number of objects of the stress test scenes) without editing the code, and a smoke test for each test scene.
- Materials as assets: each one is described by a `<name>.material.json` file (its name, the name of its shader, its parameters, its textures, and whether its shader supports instancing), loaded by the new `Material_Library` from `resources/materials`. Meshes without a material of their own use the "Default" material.
- Texture slots in shader descriptors (e.g., the albedo, metallic, and opacity maps of the lit shader), filled by the textures of each material.
- Reflective and refractive materials ("Chrome", "Mirror", "Glass", and "Dynamic Glass"), whose shaders sample an environment map: the skybox, or a cubemap captured every frame from the position of each object, as set by the material's `environment` field. The dynamic cubemaps are created and released automatically for the objects that use such materials.
- Imported models create a material for each of their materials with textures (added to the material library, so that it can also be assigned to other objects).
- Material assignment at runtime from the Properties window, and material parameters that each object can override for itself.
- Material Editor window in the App (opened from the Creation window, or from the material of the selected object): it creates materials (with any shader of the library, starting with its parameters) and duplicates them, renames them, edits their parameters, textures, and environment map while the scene is rendered with them, marks those with unsaved changes, and saves them to (or reloads them from) their descriptor files.
- Saving, reloading, duplication, and renaming of materials in `Material_Library`, and the material parameters of a shader, found from the active uniforms of its program (`Shader::get_uniforms`).
- Shader programs as assets: each one is described by a `<name>.shader.json` file (its name and the files of its stages), loaded by the new `Shader_Library` from `resources/shaders` (applications can load their own directories).
- Shader reloading at runtime from the Debug window (all shaders or one by one): a shader that fails to compile keeps its previous program.
- nlohmann/json dependency (vcpkg `nlohmann-json`, Ubuntu `nlohmann-json3-dev`).
- `#include` directive for shaders, resolved by the engine's shader preprocessor (with `#line` directives, so that compiler messages show the line numbers of the original files).

### Changed

- The lit shaders share the lights and the lighting functions through `resources/shaders/include/lighting.glsl`.
- Render passes can have a different color format per attachment.
- The renderer takes the shaders of its passes from the shader library by name, and shaders are no longer scene nodes of the node manager.
- The renderer draws every model with the shader and parameters of its material (instead of choosing the shader by node type, with a hard-coded shininess), and instanced rendering groups the objects by geometry and material.
- The albedo color of each object reaches the lit shader as `u_object_albedo`, separate from the material parameters (`u_material`): it tints the material's textures, and its alpha is the object's opacity. Imported models are white by default, and their color can be edited like the color of shapes.
- A single lit shader (`lit.vert.glsl` and `lit.frag.glsl`) replaces the shape and Assimp model shaders, so shapes can also have textured materials, and textured objects can be drawn with instancing.
- The scenes of the App moved from `main.cpp` to `app/scenes/` (the example scene, the reflective and refractive test scenes, and the stress test scenes), with a registry that names them.
- The textures of meshes belong to their materials: a mesh has an optional material, used unless its node has a material of its own.
- Only the opacity of textures discards nearly transparent fragments (below 0.1); the opacity of an object always blends.
- The parameters of a shader that a material does not set are drawn with their zero values, instead of the values of the previous material drawn with the same shader.

### Fixed

- The entry of the Material combo box of the Properties window that removes the material of an object was named "Default", like the default material, so both entries had the same ImGui identifier (reported as a conflict in Debug builds, and a click could select the other entry). It is now named "Mesh Materials".
- The opacity maps of imported models were sampled from the texture unit of the albedo map.
- The capture of dynamic environment maps skipped the own meshes of composite models and could draw their children twice.
- The dynamic environment maps were captured every frame but never displayed (the reflective and refractive shaders were not used in the main pass), and they were captured from the local position of the objects instead of their world position.
- The reflective and refractive test scenes did not load their imported model on case-sensitive file systems (Linux), since the name of its file was misspelled.
- Textures were never released from the GPU: they are now released when their last owner (e.g., a material or the skybox) releases them.

### Removed

- The hard-coded list of built-in shaders, `Renderer::set_shader_by_name()`, and `Core::compile_shaders()` (replaced by the shader library).
- `Renderer::register_model_for_dynamic_env_map_capture()` and `Renderer::unregister_model_for_dynamic_env_map_capture()` (replaced by the dynamic environment materials).
- The shape and Assimp model shaders (replaced by the lit shader), the textures of meshes and `Mesh::bind_textures()`, and `Shape_Model::add_texture_data()` (replaced by the textures of materials).

## [0.9.0] - 2026-10-08

### Added

- Cross-platform CMake presets: Ninja/MSVC, Visual Studio 2022 and 2026, MinGW Makefiles with GCC, and Ninja with GCC or Clang on Linux.
- `vcpkg.json` manifest with a pinned baseline: vcpkg installs GLFW, GLM, and Assimp for every Windows preset.
- `Engine::Core` and `Engine::Platform` libraries, consumable by other CMake projects with `add_subdirectory()`, with a `Gui_Layer` interface for application GUIs.
- Runtime lookup of the resources directory, so the App runs from any working directory.
- GitHub Actions CI: formatting check, Linux (GCC and Clang), Windows (MSVC and MinGW), unit tests, App smoke test, and consumer test.
- GitHub Actions release workflow, which attaches a Windows package of the App (with its DLLs, the MSVC runtime, and the resources) to each published release.
- GoogleTest unit tests for `String_Utils`, `File_System_Utils`, `Random`, `Bounding_Box`, and `Frustum`.
- `--frames N` command-line option to render a fixed number of frames and exit.
- Frustum culling, with the `Bounding_Box` and `Frustum` classes.
- Geometries shared between meshes with identical data (`Mesh_Geometry`).
- Instanced rendering of opaque shapes sharing a geometry.
- Shader storage buffer with all the lights of the scene.
- Debug window statistics (drawn, culled, and instanced objects, mesh geometries, and lights) and toggles for frustum culling and instancing.
- `.clang-format` and `.editorconfig` files, applied to the whole code base.
- Changelog and development workflow documentation.

### Changed

- OpenGL 4.5 core profile (previously 4.2).
- The GLFW window, input, and ImGui integration moved from the App to the Platform library, and the built-in shaders and resource lookup from the App to the Core.
- `CMakePresets.json` replaces `CMakeSettings.json` for Visual Studio.
- The `Random` class uses a seedable `std::mt19937` generator with correct distributions.
- The main loop renders a few frames at startup and after each event before waiting for the next one.

### Removed

- The MSVC binaries and headers of GLFW, GLM, and Assimp bundled in `external/`, the `ENGINE_USE_BUNDLED_DEPS` option, and the custom `Find*.cmake` modules.
- `imgui.ini` from version control.
- The limit of three lights per type.

### Fixed

- Point light input mismatch in the Assimp model shader.
- Geometry shader source not passed to OpenGL.
- OpenGL objects released after the window was destroyed at shutdown.
- GPU buffers of meshes never released.
- Corrupted lighting with more than three lights of a type.
- Incomplete window and GUI at startup until the first user input.
- Random positions ignoring the minimum distance and spreading only on one side of the target.
- Clang build errors and warnings, and missing standard includes.

## [0.8.1] - 2026-06-29

See the [v0.8.1 release](https://github.com/emartg/opengl-graphics-engine/releases/tag/v0.8.1). Earlier versions are documented in their [GitHub releases](https://github.com/emartg/opengl-graphics-engine/releases).

[Unreleased]: https://github.com/emartg/opengl-graphics-engine/compare/v0.9.0...HEAD
[0.9.0]: https://github.com/emartg/opengl-graphics-engine/compare/v0.8.1...v0.9.0
[0.8.1]: https://github.com/emartg/opengl-graphics-engine/releases/tag/v0.8.1
