
# Build Instructions

This document provides detailed steps to compile and run the OpenGL Graphics Engine.

## Prerequisites

- **Operating System:** Windows (tested on Windows 11) or Linux (tested on Ubuntu 24.04). macOS is not officially tested.
- **Compiler:** MSVC from Visual Studio 2022/2026, or GCC from a MinGW installation on Windows; GCC or Clang on Linux.
- **CMake:** ≥ 3.21 (≥ 3.25 to use the presets in `CMakePresets.json`).
- **VS Code extensions:** CMake Tools and C/C++.
- **Git:** To clone the repository (optional if you download the source).

## Dependency Resolution

The engine uses CMake's `find_package` with custom `Find*.cmake` scripts located in `/cmake/modules` for the following libraries:

- GLFW
- GLM
- Assimp

For OpenGL (GLAD), `stb_image`, and ImGui, the required files are either generated (GLAD) or included directly as headers (stb_image, ImGui backends) in the source tree.

The default MSVC presets use the repository's matching dependencies in `external/`. The `mingw-gcc-vcpkg-*` presets use GCC and the `x64-mingw-dynamic` vcpkg triplet. vcpkg is a package manager and CMake toolchain integration; it is not a compiler.

For MinGW, install MinGW with GCC and install the dependencies into the same vcpkg installation:

```powershell
vcpkg install glfw3:x64-mingw-dynamic assimp:x64-mingw-dynamic glm:x64-mingw-dynamic
```

The repository does not contain machine-specific compiler or vcpkg paths. Configure the paths as user environment variables, replacing the examples with the locations on your machine:

```powershell
[Environment]::SetEnvironmentVariable("MINGW_ROOT", "C:\path\to\mingw64", "User")
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\path\to\vcpkg", "User")
```

Restart VS Code after changing persistent environment variables. Verify the values in a new, ordinary PowerShell terminal:

```powershell
$env:MINGW_ROOT
$env:VCPKG_ROOT
Get-Command gcc
Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

Use an ordinary PowerShell terminal for the MinGW preset. A Visual Studio Developer PowerShell can inject a different `VCPKG_ROOT` into the current process. If that happens, correct it before configuring:

```powershell
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
```

## Build directories and generators

Each CMake build directory belongs to exactly one generator/toolchain/configuration combination. The default presets use Ninja with MSVC:

- Debug: `out/build/ninja-msvc-debug`
- Release: `out/build/ninja-msvc-release`

The generator is part of the directory name because CMake does not allow a build tree to change generators after configuration. A Visual Studio generator, Makefiles, or another toolchain must use a different directory, for example `out/build/vs2022-msvc-debug` or `out/build/mingw-gcc-vcpkg-debug`.

`out/install/<configuration>` is an optional installation prefix. It is populated only by `cmake --install` and is separate from the build tree; normal compilation does not require it. The `.vs` folder contains Visual Studio's local metadata and can remain in place.

## Building with VS Code (recommended)

The repository contains CMake presets for the supported Windows workflow: MSVC x64, Ninja, and the bundled libraries in `external/`. Both VS Code and Visual Studio use the same canonical output directories. Do not configure or build from both IDEs at the same time.

1. Install the **CMake Tools** and **C/C++** extensions.
2. Open the repository root in VS Code.
3. Open the Command Palette with `Ctrl+Shift+P` and run **CMake: Select Configure Preset**.
4. Select the desired preset: Ninja/MSVC, Visual Studio 2022/2026, or MinGW Makefiles with GCC and vcpkg. Each preset names its generator, compiler, dependency provider, and configuration.
5. Run **CMake: Configure**. CMake Tools may configure automatically when the folder opens; run it manually after changing presets or clearing a cache.
6. Run **CMake: Set Build Target** and select `App`.
7. Run **CMake: Build**. This builds `GLAD`, `STB_IMAGE`, `Core`, and `App`.
8. For Debug, open **Run and Debug**, select `Debug App (CMake)`, and press `F5`. For Release, use **CMake: Build** and launch the resulting executable from its `bin` directory.

The generated executables are:

- Debug: `out/build/ninja-msvc-debug/bin/App.exe`
- Release: `out/build/ninja-msvc-release/bin/App.exe`

Use **CMake: Delete Cache and Reconfigure** if changing generators, compilers, or dependency locations. Do not use the generated active-file compiler tasks; they compile only one source file and do not link the project.

From a Visual Studio Developer PowerShell, the equivalent commands are:

```powershell
# Configure and build in a fresh preset-specific directory
cmake --preset ninja-msvc-debug
cmake --build --preset ninja-msvc-debug

# Run with bin as the working directory
out\build\ninja-msvc-debug\bin\App.exe
```

The VS Code launch configuration uses the CMake-selected target and sets the working directory to the generated `bin` directory. This is important because the engine loads shaders, models, fonts, and textures using paths relative to the working directory.

The `external/` directory is the authoritative dependency source for this workflow. Do not combine its Assimp or GLFW libraries with headers from vcpkg or another installation. The current checked-in `external/dlls/assimp-vc143-mtd.dll` is a Debug DLL; a matching Release DLL is required before using the Release preset.

## Building with Visual Studio

1. **Clone or download** the source code.

2. **Open the root folder** in Visual Studio. Visual Studio will automatically detect the CMake project.

3. **Configure CMake**:
   - VS will run CMake configuration automatically.
   - You can manually trigger it via `Project > Configure Cache`.
   - Select `Ninja MSVC x64 Debug` or `Ninja MSVC x64 Release` in the CMake configuration dropdown.
   - Visual Studio can use the same Ninja configurations as VS Code because the generator is independent of the IDE.

4. **Build**:
   - Build the entire solution: `Build > Build All`.
   - Alternatively, build only the `App` target.

5. **Run**:
   - Set `App.exe` as the startup item (if not already).
   - Press `F5` (Debug) or `Ctrl+F5` (Run without debugging).

Visual Studio reads these configurations from `CMakeSettings.json`. Do not configure or build in VS Code at the same time, because both IDEs share the same generated CMake files.

## MinGW with GCC and vcpkg

The supported MinGW workflow uses the `MinGW Makefiles` generator, GCC, and vcpkg. The preset name follows that combination: `mingw-gcc-vcpkg-debug` or `mingw-gcc-vcpkg-release`.

```powershell
# Make GCC discoverable in this terminal.
$env:Path = "$env:MINGW_ROOT\bin;$env:Path"
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")

# Configure and build Debug.
cmake --fresh --preset mingw-gcc-vcpkg-debug
cmake --build --preset mingw-gcc-vcpkg-debug --parallel
```

```powershell
# Optional: use the release preset for Release builds.
cmake --fresh --preset mingw-gcc-vcpkg-release
cmake --build --preset mingw-gcc-vcpkg-release --parallel
```

The preset expands `$env{VCPKG_ROOT}` when configuring, so no developer-specific path is committed to the repository. Use `cmake --fresh` after changing compiler or dependency locations because CMake caches the toolchain file. Do not reuse a build directory configured for a different generator or compiler.

For Visual Studio 2026, use the generator name reported by `cmake --help` and a directory such as `out/build/vs2026-msvc-debug`. Do not reuse a directory configured for another generator or compiler.

## Linux with GCC or Clang

The Linux presets use Ninja and the system packages for GLFW, Assimp, and GLM
(the binaries in `external/` are built with MSVC, so `ENGINE_USE_BUNDLED_DEPS` is disabled).
On Ubuntu/Debian, install the toolchain and the dependencies with:

```bash
sudo apt install build-essential clang ninja-build cmake libglfw3-dev libassimp-dev libglm-dev libgl-dev
```

The configure, build, and run with the GCC or Clang presets

```bash
# Configure and build Debug with GCC (use ninja-clang-debug for Clang)
cmake --preset ninja-gcc-debug
cmake --build ninja-gcc-debug

# Run with bin as the working directory
cd out/build/ninja-gcc-debug/bin && ./App
```

The Release presets are `ninja-gcc-release` and `ninja-clang-release`. The presets
are only listed on the OS they apply to (Windows presets on Windows, Linux presets on Linux),
so VS Code and `cmake --list-presets` only show usable configurations.

From Windows, the Linux build can be tested through WSL2 (Windows Subsystem for Linux):
install Ubuntu with `wsl --install`, clone the repository inside the WSL file system (e.g., `~/scr`),
and follow the steps above. WSLg displays the window on the Windows desktop, and the OpenGL context is
provided by Mesa (hardware-accelerated through the D3D12 backend, or by the `llvmpipe`software renderer).
VS Code can open the WSL folder directly with the **WSL** extension (`code.` from the WSL terminal).

## Using the Engine from Another CMake Project

The Engine can be consumed by another CMake project (e.g., a simulator that includes this repository as a Git submodule)
through `add_subdirectory()`. In that case, only the `Core` library is built by default (`ENGINE_BUILD_APP`defaults
to `OFF` when the Engine is not the top-level project), and the consumer keeps
its own C++ standard, build type, and output directories.

```cmake
# Consumer CMakeLists.txt
add_subdirectory(Engine) # path to the Engine sources (e.g., a Git submodule)

add executable(My_App main.cpp)
target_link_libraries(My_App PRIVATE Engine::Core)
```

`Engine::Core` propagates its include directories and dependencies, so the consumer includes the Engine headers relative
to the Engine root:

```cpp
#include "core/Core.h"
#include "core/camera/Camera.h"
```

The Engine root directory is exposed to consumers in the `ENGINE_ROOT_DIR` variable (e.g., locate the Engine's resources).

## Building with CMake Presets from the Command Line

The preset workflow can also be run entirely from a Visual Studio Developer PowerShell:

```powershell
# Configure with the Ninja/MSVC x64 Debug preset
cmake --preset ninja-msvc-debug

# Build App and its dependencies
cmake --build --preset ninja-msvc-debug

# Run
out/build/ninja-msvc-debug/bin/App.exe
```

For Release builds, use `ninja-msvc-release` only after providing a matching Release Assimp DLL.

## Building Directly with CMake

The project also supports the traditional direct CMake workflow. Run these commands from a Visual Studio Developer PowerShell so that `cl.exe`, the Windows SDK, and the MSVC libraries are available:

```powershell
# Configure a fresh MSVC x64 Release build with Ninja
cmake -S . -B out/build/manual-ninja-msvc-release -G Ninja `
   -DCMAKE_BUILD_TYPE=Release `
   -DCMAKE_C_COMPILER=cl `
   -DCMAKE_CXX_COMPILER=cl

# Build App and its dependencies
cmake --build out/build/manual-ninja-msvc-release --parallel

# Run from the generated bin directory
out\build\manual-ninja-msvc-release\bin\App.exe
```

For a Debug build, replace `Release` with `Debug` and use a separate directory, for example `out/build/manual-ninja-msvc-debug`. With Ninja, `--config Release` is optional because Ninja is a single-configuration generator; `CMAKE_BUILD_TYPE` selects the configuration during configure.

Do not reuse a build directory previously configured with MinGW, Visual Studio, or another generator. Use a new directory or remove the old CMake cache first.

## Clean Build

To clean all generated files:

```bash
cmake --build out/build/ninja-msvc-debug --target clean
```

To delete the entire build directory (nuclear option):

```powershell
# Remove all generated CMake output, including Debug, Release, and manual builds
Remove-Item -Recurse -Force out\build
```

## Troubleshooting

- **GLFW or Assimp not found with MinGW:** Check that `VCPKG_ROOT` points to the vcpkg installation containing `x64-mingw-dynamic`, not a Visual Studio-only vcpkg installation. Run `vcpkg install glfw3:x64-mingw-dynamic assimp:x64-mingw-dynamic glm:x64-mingw-dynamic`, then run `cmake --fresh --preset mingw-gcc-vcpkg-debug`.
- **Environment value looks correct but CMake uses another path:** The current terminal may have inherited a stale `VCPKG_ROOT`. Run `$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")`, or close and reopen VS Code.
- **GLAD errors:** The `glad.c` and `glad.h` are generated in `core/third_party/glad/`. If they are missing, regenerate them from the [GLAD service](https://glad.dav1d.de/) using OpenGL 4.2 Core.
- **Assimp DLL missing:** If running the executable fails with a missing `assimp-vc143-mtd.dll`, copy it from your Assimp installation into the same directory as `App.exe`, or add its folder to your `PATH`.

For additional help, consult the main [README](../README.md) or raise an issue in the repository (if you have one).
