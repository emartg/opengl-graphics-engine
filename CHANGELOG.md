# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html) (before v1.0.0, minor versions may include breaking changes). The detailed notes of each version are published in its [GitHub release](https://github.com/emartg/opengl-graphics-engine/releases).

## [Unreleased]

### Added

- Shader programs as assets: each one is described by a `<name>.shader.json` file (its name and the files of its stages), loaded by the new `Shader_Library` from `resources/shaders` (applications can load their own directories).
- Shader reloading at runtime from the Debug window (all shaders or one by one): a shader that fails to compile keeps its previous program.
- nlohmann/json dependency (vcpkg `nlohmann-json`, Ubuntu `nlohmann-json3-dev`).
- `#include` directive for shaders, resolved by the engine's shader preprocessor (with `#line` directives, so that compiler messages show the line numbers of the original files).

### Changed

- The lit shaders share the lights and the lighting functions through `resources/shaders/include/lighting.glsl`.
- Render passes can have a different color format per attachment.
- The renderer takes the shaders of its passes from the shader library by name, and shaders are no longer scene nodes of the node manager.

### Removed

- The hard-coded list of built-in shaders, `Renderer::set_shader_by_name()`, and `Core::compile_shaders()` (replaced by the shader library).

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
