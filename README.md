
# OpenGL 4.5 based Graphics Engine

[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-4.5-red.svg)](https://www.opengl.org/)
[![CMake](https://img.shields.io/badge/CMake-≥3.20-brightgreen.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A **modular, extensible 3D graphics engine** built from scratch with OpenGL 4.5. Initially developed as a Bachelor's Thesis (TFG), it serves as a lightweight prototyping platform for real-time rendering, featuring a composite scene graph, lighting, model import, an immediate-mode GUI editor, among other functionalities.

---

## Features

- **Scene Graph & Composite Pattern** - Hierarchical node system with transform inheritance, grouping, and drag-and-drop reparenting in the GUI.
- **Lighting** - Point lights, spotlights, and directional lights with editable parameters (color, intensity, attenuation, cut-off angles) and visual gizmos.
- **Model Import** - Load 3D models (`.obj`, `.gltf`, `.fbx`, etc.) via **Assimp**.
- **Skyboxes** - HDR environment maps and custom cubemap loading (six face images).
- **Reflection & Refraction** - Dynamic per-object environment maps with ping-pong buffering to avoid recursion.
- **Transparency & Blending** - Alpha channel support with back-to-front sorting for correct blending.
- **Color Picking & Outlining** - Click-to-select objects using off-screen color encoding; selected objects are highlighted with a depth-aware outline.
- **ImGui Editor** - Rich GUI panels:
  - *Scene Graph* - tree view with drag-and-drop reparenting.
  - *Properties* - real-time editing of transforms, color, opacity, light parameters, etc.
  - *Debug* - visualisation modes (Normal, Inverted Colors, Picking Colors, Grid Overlay) and performance stats.
  - *Creation* - instantiate primitives (cube, sphere, cylinder, cone, plane) or import models/skyboxes.
- **Efficient Event-Driven Loop** - Blocks on idle using `glfwWaitEvents()`, reducing CPU usage to near zero when inactive.

---

## Architecture Overview

The engine is split into two CMake modules:

| Module | Description |
| ------ | ----------- |
| **`core`** | Static library containing all rendering logic, scene management, shaders, and resource handling. Dependencies: OpenGL, GLAD, GLM, stb_image, Assimp. |
| **`app`** | Executable that provides the window, input handling, and GUI (GLFW, ImGui). It instantiates the `core` and demonstrates its usage. |

This separation allows the `core` to be reused in other projects without pulling GUI or windowing dependencies.

For a detailed class diagram and pipeline description, see the [Architecture Guide](docs/architecture.md).

---

## Dependencies

| Library | Version | Purpose |
| ------- | ------- | ------- |
| OpenGL | 4.5 Core | Graphics API |
| GLFW | 3.4 | Window creation, input, context |
| GLAD | 0.1.36 | OpenGL extension loading |
| GLM | 0.9.8.5 | Vector/matrix mathematics |
| stb_image | 2.30 | Image loading (`.jpg`, `.png`, etc.) |
| Dear ImGui | 1.91.6 | Immediate-mode GUI |
| Assimp | 6.0.2 | 3D model import |

CMake (≥3.20) with Ninja generator is recommended, but any generator works.

---

## Building

See the detailed [Build Instructions](docs/build_instructions.md) for platform-specific steps. In short:

```bash
# Configure the default Ninja/MSVC x64 Debug preset
cmake --preset ninja-msvc-debug

# Build App and its dependencies
cmake --build --preset ninja-msvc-debug

# Run (resources are located automatically, whatever the working directory)
./out/build/ninja-msvc-debug/bin/App.exe
```

In VS Code, install **CMake Tools** and **C/C++**, select the `Ninja MSVC x64 Debug` or `Ninja MSVC x64 Release` configure preset, configure the project, set `App` as the build target, and use **CMake: Build** or the `Debug App (CMake)` launch configuration. Visual Studio can use the same Ninja configurations through `CMakeSettings.json`, while native Visual Studio or other generators should use their own descriptive build directory. See the [Build Instructions](docs/build_instructions.md) for the complete IDE and CLI workflows.

For MinGW, use the `mingw-gcc-vcpkg-debug` or `mingw-gcc-vcpkg-release` preset. These presets use the MinGW Makefiles generator, GCC, and vcpkg's `x64-mingw-dynamic` triplet. Before configuring, install the matching packages and set `MINGW_ROOT` and `VCPKG_ROOT` as user environment variables:

```powershell
vcpkg install glfw3:x64-mingw-dynamic assimp:x64-mingw-dynamic glm:x64-mingw-dynamic
[Environment]::SetEnvironmentVariable("MINGW_ROOT", "C:\path\to\mingw64", "User")
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\path\to\vcpkg", "User")
```

Restart VS Code after changing these variables. In a new ordinary PowerShell terminal, configure and build with:

```powershell
$env:Path = "$env:MINGW_ROOT\bin;$env:Path"
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
cmake --fresh --preset mingw-gcc-vcpkg-debug
cmake --build --preset mingw-gcc-vcpkg-debug --parallel
```

Use the release preset for Release builds. If CMake reports that GLFW or Assimp is missing, verify that `VCPKG_ROOT` points to the vcpkg installation containing `x64-mingw-dynamic`, then reconfigure with `cmake --fresh`. Avoid using a Visual Studio Developer PowerShell for this preset if it injects a different `VCPKG_ROOT`.

Alternatively, build directly with CMake from a Visual Studio Developer PowerShell. Keep manual builds under `out/build/` and use a fresh directory when changing compilers or generators:

```powershell
cmake -S . -B out/build/manual-ninja-msvc-release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER=cl `
  -DCMAKE_CXX_COMPILER=cl
cmake --build out/build/manual-ninja-msvc-release --parallel
out/build/manual-ninja-msvc-release\bin\App.exe
```

For Debug, change `Release` to `Debug` and use a separate directory such as `out/build/manual-ninja-msvc-debug`. The preset and direct CLI workflows are equivalent; presets simply keep the default configuration choices in the repository.

On Linux, install GLFW, Assimp, and GLM from the system package manager
and use the GCC or Clang presets (`ninja-gcc-debug`, `ninja-clang-debug`, and their Release variants):

```bash
sudo apt install build-essential clang ninja-build cmake libglfw3-dev libassimp-dev libglm-dev libgl-dev
cmake --preset ninja-gcc-debug
cmake --build --preset ninja-gcc-debug
./out/build/ninja-gcc-debug/bin/App
```

The Engine can also be consumed by other CMake projects (e.g., as a Git submodule)
with `add_subdirectory()` and the `Engine::Core` target; see [Using the Engine from
Another CMake Project](docs/build_instructions.md#using-the-engine-from-another-cmake-project).

---

## Usage

- **Camera** - Drag with right mouse button to orbit, scroll to zoom.
- **Select** - Left-click on any renderable object to select it (outline appears). Double-click to select the parent group.
- **GUI Panels** - All panels are draggable, resizable, and collapsible.
  - *Scene Graph* - drag nodes to reparent; drop on the bottom area to make the node a root.
  - *Properties* - modify transforms, color, opacity, light parameters, etc.
  - *Creation* - create new lights, primitives, or import models/skyboxes.
  - *Debug* - switch visualisation modes and reset GUI layout.

---

## Screenshots

| Feature | Example Screenshot |
| ------- | ------------------ |
| **Window - Scene & GUI Editor** | <div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"> <img src="docs/images/gui/layout/gui_layout_1.png" alt="gui_layout_1" title="Initial window and GUI with the example scene and the red cube selected" style="height:400px; width:auto;" /> </div> |
| **Blending & Transparency** | <div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"> <img src="docs/images/blending/blending_3.png" alt="blending_3" title="Blending and transparency demonstration in the example scene" style="height:240px; width:auto;" /> <img src="docs/images/blending/blending_4.png" alt="blending_4" title="Blending and transparency demonstration in the example scene" style="height:240px; width:auto;" /> </div> |
| **Outlining** | <div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"> <img src="docs/images/selection/selection_1.png" alt="outlining_1" title="Outlining over red cube with transparent teapot" style="height:240px; width:auto;" /> <img src="docs/images/selection/selection_3.png" alt="outlining_3" title="Outlining over group selection demonstration" style="height:240px; width:auto;" /> </div> |
| **Reflection** | <div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"><img src="docs/images/reflection/reflection_3.png" alt="reflection_3" title="Matte teapot vs reflective plane" style="height:240px; width:auto;" /> <img src="docs/images/reflection/reflection_6.png" alt="reflection_6" title="Reflective teapot vs reflective plane" style="height:240px; width:auto;" /> </div> |
| **Refraction** | <div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"><img src="docs/images/refraction/refraction_1.png" alt="refraction_1" title="Matte teapot vs refractive plane" style="height:240px; width:auto;" /> <img src="docs/images/refraction/refraction_4.png" alt="refraction_4" title="Refractive teapot vs refractive plane" style="height:240px; width:auto;" /> </div> |

See other captures in the subfolder [Images](docs/images/).

---

## Current Status

This engine is a **beta prototype** (v0.8.0) - functional and usable, but not production-ready. Known limitations include:

- No instanced rendering or frustum culling (performance drops with >1000 opaque objects).
- Transparent objects become expensive (>500 objects).
- Dynamic environment maps are computationally heavy (>10 reflective objects).
- Shader/material assignment is hard-coded (no data-driven material system).
- Limited animation and material import from Assimp.

See the full [Limitations and Known Issues](docs/limitations.md) document for a comprehensive list.

---

## Future Roadmap

Short-term optimisations (instancing, frustum culling, order-independent transparency) and longer-term features (data-driven materials, full Assimp support, multi-camera, post-processing, PBR) are outlined in the [Future Work](docs/future_work.md) document.

---

## License

This project is distributed under the **MIT License**. See the [LICENSE](LICENSE) file for details. All images and documentation are likewise covered by the MIT License.
