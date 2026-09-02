
# Build Instructions

This document provides detailed steps to compile and run the OpenGL Graphics Engine.

## Prerequisites

- **Operating System:** Windows (tested on Windows 11). Linux/macOS are not officially tested but CMake configuration is prepared for them.
- **Compiler:** Visual Studio 2022 (with C++20 support) or any C++20 compiler.
- **CMake:** ≥ 3.20.
- **VS Code extensions:** CMake Tools and C/C++.
- **Git:** To clone the repository (optional if you download the source).

## Dependency Resolution

The engine uses CMake's `find_package` with custom `Find*.cmake` scripts located in `/cmake/modules` for the following libraries:

- GLFW
- GLM
- Assimp

For OpenGL (GLAD), `stb_image`, and ImGui, the required files are either generated (GLAD) or included directly as headers (stb_image, ImGui backends) in the source tree.

The build will attempt to locate these dependencies on your system. If not found, you may need to install them manually (e.g., via vcpkg, Conan, or direct download) and set the appropriate `CMAKE_PREFIX_PATH`.

## Canonical build directories

Both VS Code and Visual Studio use the same CMake output directories:

- Debug: `out/build/msvc-debug`
- Release: `out/build/msvc-release`

The IDEs must not configure or build at the same time, because they share the generated CMake cache and Ninja files. The `.vs` folder contains Visual Studio's local metadata and can remain in place.

## Building with VS Code (recommended)

The repository contains CMake presets for the supported Windows workflow: MSVC x64, Ninja, and the bundled libraries in `external/`. Both VS Code and Visual Studio use the same canonical output directories. Do not configure or build from both IDEs at the same time.

1. Install the **CMake Tools** and **C/C++** extensions.
2. Open the repository root, `C:\Dev\XRaySim\Engine`, in VS Code.
3. Open the Command Palette with `Ctrl+Shift+P` and run **CMake: Select Configure Preset**.
4. Select `MSVC x64 Debug` or `MSVC x64 Release`.
5. Run **CMake: Configure**. CMake Tools may configure automatically when the folder opens; run it manually after changing presets or clearing a cache.
6. Run **CMake: Set Build Target** and select `App`.
7. Run **CMake: Build**. This builds `GLAD`, `STB_IMAGE`, `Core`, and `App`.
8. For Debug, open **Run and Debug**, select `Debug App (CMake)`, and press `F5`. For Release, use **CMake: Build** and launch the resulting executable from its `bin` directory.

The generated executables are:

- Debug: `out/build/msvc-debug/bin/App.exe`
- Release: `out/build/msvc-release/bin/App.exe`

Use **CMake: Delete Cache and Reconfigure** if changing generators, compilers, or dependency locations. Do not use the generated active-file compiler tasks; they compile only one source file and do not link the project.

From a Visual Studio Developer PowerShell, the equivalent commands are:

```powershell
# Configure and build in a fresh preset-specific directory
cmake --preset msvc-debug
cmake --build --preset msvc-debug

# Run with bin as the working directory
out\build\msvc-debug\bin\App.exe
```

The VS Code launch configuration uses the CMake-selected target and sets the working directory to the generated `bin` directory. This is important because the engine loads shaders, models, fonts, and textures using paths relative to the working directory.

The `external/` directory is the authoritative dependency source for this workflow. Do not combine its Assimp or GLFW libraries with headers from vcpkg or another installation. The current checked-in `external/dlls/assimp-vc143-mtd.dll` is a Debug DLL; a matching Release DLL is required before using the Release preset.

## Building with Visual Studio (Ninja Generator)

1. **Clone or download** the source code.

2. **Open the root folder** in Visual Studio. Visual Studio will automatically detect the CMake project.

3. **Configure CMake**:
   - VS will run CMake configuration automatically.
   - You can manually trigger it via `Project > Configure Cache`.
   - Select `MSVC x64 Debug` or `MSVC x64 Release` in the CMake configuration dropdown.
   - Both configurations use the canonical `out/build/msvc-debug` and `out/build/msvc-release` directories.

4. **Build**:
   - Build the entire solution: `Build > Build All`.
   - Alternatively, build only the `App` target.

5. **Run**:
   - Set `App.exe` as the startup item (if not already).
   - Press `F5` (Debug) or `Ctrl+F5` (Run without debugging).

Visual Studio reads these configurations from `CMakeSettings.json`. Do not configure or build in VS Code at the same time, because both IDEs share the same generated CMake files.

## Building with CMake Presets from the Command Line

The preset workflow can also be run entirely from a Visual Studio Developer PowerShell:

```powershell
# Configure with the MSVC x64 Debug preset
cmake --preset msvc-debug

# Build App and its dependencies
cmake --build --preset msvc-debug

# Run
out/build/msvc-debug/bin/App.exe
```

For Release builds, use `msvc-release` only after providing a matching Release Assimp DLL.

## Building Directly with CMake

The project also supports the traditional direct CMake workflow. Run these commands from a Visual Studio Developer PowerShell so that `cl.exe`, the Windows SDK, and the MSVC libraries are available:

```powershell
# Configure a fresh MSVC x64 Release build with Ninja
cmake -S . -B out/build/manual-msvc-release -G Ninja `
   -DCMAKE_BUILD_TYPE=Release `
   -DCMAKE_C_COMPILER=cl `
   -DCMAKE_CXX_COMPILER=cl

# Build App and its dependencies
cmake --build out/build/manual-msvc-release --parallel

# Run from the generated bin directory
out\build\manual-msvc-release\bin\App.exe
```

For a Debug build, replace `Release` with `Debug` and use a separate directory, for example `out/build/manual-msvc-debug`. With Ninja, `--config Release` is optional because Ninja is a single-configuration generator; `CMAKE_BUILD_TYPE` selects the configuration during configure.

Do not reuse a build directory previously configured with MinGW, Visual Studio, or another generator. Use a new directory or remove the old CMake cache first.

## Clean Build

To clean all generated files:

```bash
cmake --build out/build/msvc-debug --target clean
```

To delete the entire build directory (nuclear option):

```powershell
# Remove all generated CMake output, including Debug, Release, and manual builds
Remove-Item -Recurse -Force out\build
```

## Troubleshooting

- **Missing dependencies:** If CMake fails to find a library, ensure it is installed and its path is added to `CMAKE_PREFIX_PATH`. For example: `-DCMAKE_PREFIX_PATH="C:/path/to/libs"`.
- **GLAD errors:** The `glad.c` and `glad.h` are generated in `core/third_party/glad/`. If they are missing, regenerate them from the [GLAD service](https://glad.dav1d.de/) using OpenGL 4.2 Core.
- **Assimp DLL missing:** If running the executable fails with a missing `assimp-vc143-mtd.dll`, copy it from your Assimp installation into the same directory as `App.exe`, or add its folder to your `PATH`.

For additional help, consult the main [README](../README.md) or raise an issue in the repository (if you have one).
