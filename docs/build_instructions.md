
# Build Instructions

This document provides detailed steps to compile and run the OpenGL Graphics Engine.

## Prerequisites

- **Operating System:** Windows (tested on Windows 11) or Linux (tested on Ubuntu 26.04). macOS is not supported, since it only provides OpenGL up to 4.1.
- **GPU and drivers:** OpenGL 4.5 core profile support (any NVIDIA, AMD, or Intel GPU with current drivers on Windows or Linux, or a recent Mesa on Linux, including the `llvmpipe` software renderer).
- **Compiler:** MSVC from Visual Studio 2022/2026, or GCC from a MinGW installation on Windows; GCC or Clang on Linux.
- **CMake:** ≥ 3.21 (≥ 3.25 to use the presets in `CMakePresets.json`).
- **vcpkg (Windows):** A Git clone of [vcpkg](https://github.com/microsoft/vcpkg), used by the Windows presets to build the dependencies.
- **VS Code extensions:** CMake Tools and C/C++.
- **Git:** To clone the repository (optional if you download the source).

## Dependency Resolution

The engine depends on GLFW, GLM, Assimp, and nlohmann/json, which are located with CMake's `find_package` through the CMake packages they install. OpenGL (GLAD), `stb_image`, ImGui, and ImGuiFileDialog are part of the source tree (in `external/include` and `core/`): GLAD is generated, and the others are included as sources.

The libraries are provided by:

- **Windows (every preset):** vcpkg. The dependencies are declared in the `vcpkg.json` manifest (vcpkg manifest mode), so vcpkg builds and installs them into the build directory during the first configuration, for the preset's triplet: `x64-windows` with MSVC, and `x64-mingw-dynamic` with MinGW. The manifest's `builtin-baseline` pins the versions of the packages, so every machine and the CI use the same ones. vcpkg is a package manager and CMake toolchain integration; it is not a compiler.
- **Linux:** the system packages (see [Linux with GCC or Clang](#linux-with-gcc-or-clang)).

Clone and bootstrap vcpkg once (any location works):

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\path\to\vcpkg
C:\path\to\vcpkg\bootstrap-vcpkg.bat -disableMetrics
```

The vcpkg clone must contain the `builtin-baseline` commit of `vcpkg.json`. If CMake reports that the baseline cannot be found, update the clone with `git -C $env:VCPKG_ROOT pull` and run `bootstrap-vcpkg.bat` again.

The repository does not contain machine-specific compiler or vcpkg paths. Configure the paths as user environment variables, replacing the examples with the locations on your machine (`MINGW_ROOT` is only needed for the MinGW presets):

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\path\to\vcpkg", "User")
[Environment]::SetEnvironmentVariable("MINGW_ROOT", "C:\path\to\mingw64", "User")
```

Restart VS Code after changing persistent environment variables. Verify the values in a new, ordinary PowerShell terminal:

```powershell
$env:VCPKG_ROOT
$env:MINGW_ROOT
Test-Path "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

The first configuration of each triplet builds GLFW, Assimp, and their dependencies from source, in Debug and Release, which can take 10 to 30 minutes. vcpkg stores the built packages in its binary cache (by default in `%LOCALAPPDATA%\vcpkg\archives`), so later configurations, including those of new build directories and of other presets with the same triplet, restore them in seconds. With MSVC, vcpkg also copies the required DLLs beside each executable after it is built; with MinGW, the Engine's build does it (see `engine_stage_runtime_dependencies` in [Using the Engine from Another CMake Project](#using-the-engine-from-another-cmake-project)).

The commands that use Ninja with MSVC must run in a Visual Studio Developer PowerShell, which adds `cl.exe`, the Windows SDK, and the MSVC libraries to the environment of the current terminal (VS Code's CMake Tools does it automatically for the presets, but not for its integrated terminal). The Developer PowerShell shortcut of the Start menu targets x86 by default, so enter an x64 one from any PowerShell (including VS Code's terminal) instead:

```powershell
# Enter the x64 developer environment of the latest Visual Studio, keeping the current directory
$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
Import-Module "$vsPath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation -DevCmdArguments "-arch=x64 -host_arch=x64"

# The developer environment may set VCPKG_ROOT to the vcpkg instance of Visual Studio: restore your own
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
```

The environment only applies to the current terminal, so repeat these commands in every new terminal (`Get-Command cl` shows whether it is active). The Visual Studio generators locate MSVC by themselves, so they do not need it.

CMake only reads the toolchain file when a build directory is configured for the first time. Build directories configured before the presets used vcpkg (or without `VCPKG_ROOT`) must be reconfigured from scratch: run **CMake: Delete Cache and Reconfigure** in VS Code, or `cmake --fresh --preset <preset>`, or delete the build directory.

## Build directories and generators

Each CMake build directory belongs to exactly one generator/toolchain/configuration combination. The default presets use Ninja with MSVC:

- Debug: `out/build/ninja-msvc-debug`
- Release: `out/build/ninja-msvc-release`

The generator is part of the directory name because CMake does not allow a build tree to change generators after configuration. A Visual Studio generator, Makefiles, or another toolchain must use a different directory, for example `out/build/vs2022-msvc-debug` or `out/build/mingw-gcc-vcpkg-debug`.

`out/install/<configuration>` is an optional installation prefix. It is populated only by `cmake --install` and is separate from the build tree; normal compilation does not require it. The `.vs` folder contains Visual Studio's local metadata and can remain in place.

## Building with VS Code (recommended)

The repository contains CMake presets for the supported Windows workflow: MSVC x64 with Ninja (or the Visual Studio generators), and the dependencies built by vcpkg (see [Dependency Resolution](#dependency-resolution)). Both VS Code and Visual Studio use the same canonical output directories. Do not configure or build from both IDEs at the same time.

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
# Use your own vcpkg clone (the Developer PowerShell may point VCPKG_ROOT to the one of Visual Studio)
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")

# Configure and build in a fresh preset-specific directory
cmake --preset ninja-msvc-debug
cmake --build --preset ninja-msvc-debug

# Run (the working directory does not matter)
out\build\ninja-msvc-debug\bin\App.exe
```

The VS Code launch configuration uses the CMake-selected target and sets the working directory to the generated `bin` directory.

## Resources Location

The engine locates its `resources` directory (shaders, fonts, models, and textures) at runtime, so the executable can be launched from any working directory. The first existing location is used, in this order:

1. The directory set explicitly with `Core::set_resources_dir()` before `Core::init()`.
2. `resources` beside the executable (e.g., a packaged build).
3. The installation layout: `<prefix>/share/EngineProject/resources`, next to `<prefix>/bin`, as created by `cmake --install`.
4. `resources` in the current working directory.
5. The `resources` directory of the source tree, whose absolute path is embedded at build time.

Resources are not copied into the build tree, so development builds use the source tree directly: changes to shaders or other resources take effect the next time the application runs, without rebuilding. The resolved directory is printed at startup (`Using resources directory: ...`).

Build directories created before this behavior may still contain an old `bin/resources` copy, which takes precedence. Delete it (or the whole build directory) so the source tree is used.

## Building with Visual Studio

1. **Clone or download** the source code.

2. **Open the root folder** in Visual Studio. Visual Studio will automatically detect the CMake project.

3. **Configure CMake**:
   - VS will run CMake configuration automatically.
   - You can manually trigger it via `Project > Configure Cache`.
   - Select `Ninja MSVC x64 + vcpkg Debug` or `Ninja MSVC x64 + vcpkg Release` in the CMake configuration dropdown.
   - Visual Studio can use the same Ninja configurations as VS Code because the generator is independent of the IDE.

4. **Build**:
   - Build the entire solution: `Build > Build All`.
   - Alternatively, build only the `App` target.

5. **Run**:
   - Set `App.exe` as the startup item (if not already).
   - Press `F5` (Debug) or `Ctrl+F5` (Run without debugging).

Visual Studio reads these configurations from `CMakePresets.json`, as VS Code does. Do not configure or build in VS Code at the same time, because both IDEs share the same generated CMake files.

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

The Linux presets use Ninja and the system packages for GLFW, Assimp, GLM, and nlohmann/json.
On Ubuntu/Debian, install the toolchain and the dependencies with:

```bash
sudo apt install build-essential clang ninja-build cmake libglfw3-dev libassimp-dev libglm-dev nlohmann-json3-dev libgl-dev
```

Then configure, build, and run with the GCC or Clang presets:

```bash
# Configure and build Debug with GCC (use ninja-clang-debug for Clang)
cmake --preset ninja-gcc-debug
cmake --build --preset ninja-gcc-debug

# Run (the working directory does not matter)
./out/build/ninja-gcc-debug/bin/App
```

The Release presets are `ninja-gcc-release` and `ninja-clang-release`. The presets
are only listed on the OS they apply to (Windows presets on Windows, Linux presets on Linux),
so VS Code and `cmake --list-presets` only show usable configurations.

From Windows, the Linux build can be tested through WSL2 (Windows Subsystem for Linux):
install Ubuntu with `wsl --install`, clone the repository inside the WSL file system (e.g., `~/src`),
and follow the steps above. WSLg displays the window on the Windows desktop, and the OpenGL context is
provided by Mesa (hardware-accelerated through the D3D12 backend, or by the `llvmpipe` software renderer).
VS Code can open the WSL folder directly with the **WSL** extension (`code .` from the WSL terminal).

## Using the Engine from Another CMake Project

The Engine can be consumed by another CMake project (e.g., an application that includes this repository as a Git submodule)
through `add_subdirectory()`. In that case, only the `Core` library is built by default (`ENGINE_BUILD_APP` defaults
to `OFF` when the Engine is not the top-level project), and the consumer keeps
its own C++ standard, build type, and output directories.

```cmake
# Consumer CMakeLists.txt
add_subdirectory(Engine) # path to the Engine sources (e.g., a Git submodule)

add_executable(My_App main.cpp)
target_link_libraries(My_App PRIVATE Engine::Core)
```

`Engine::Core` propagates its include directories and dependencies, so the consumer includes the Engine headers relative
to the Engine root:

```cpp
#include "core/Core.h"
#include "core/camera/Camera.h"
```

The Engine root directory is exposed to consumers in the `ENGINE_ROOT_DIR` variable (e.g., to locate the Engine's resources).

Applications that need a window, input, and ImGui link `Engine::Platform` instead (it includes `Engine::Core`), and implement the `Gui_Layer` interface to draw their own ImGui windows:

```cmake
target_link_libraries(My_App PRIVATE Engine::Platform)
```

```cpp
#include "core/Core.h"
#include "platform/renderer/GLFW_Renderer.h"

class My_Gui : public Gui_Layer
{
public:
    void draw() override { /* ImGui::Begin(...); ...; ImGui::End(); */ }
};

int main()
{
    Core* engine = Core::get_instance();
    GLFW_Renderer* renderer = new GLFW_Renderer();
    renderer->set_gui_layer(std::make_unique<My_Gui>());
    engine->set_renderer(renderer);
    if (engine->init())
        engine->run();
    engine->shutdown();
}
```

The Platform library is built by default (`ENGINE_BUILD_PLATFORM`); consumers that only need the Core library can disable it to skip building it and ImGui (GLFW is still located at configuration time).

The consumer must provide the Engine's dependencies (GLFW, GLM, Assimp, and nlohmann/json) as CMake packages. On Linux, the system packages are enough. On Windows, use the vcpkg toolchain, and either declare the dependencies in the consumer's own `vcpkg.json` (recommended, so that the consumer can add its own dependencies) or point vcpkg to the Engine's manifest with `-DVCPKG_MANIFEST_DIR=<engine-root>`:

```json
{
  "name": "my-app",
  "builtin-baseline": "<same baseline as the Engine's vcpkg.json>",
  "dependencies": [ "assimp", "glfw3", "glm", "nlohmann-json" ]
}
```

With MinGW, vcpkg does not copy the DLLs beside the executables, so consumers should call `engine_stage_runtime_dependencies(<target>)` (defined in `cmake/EngineDependencies.cmake`) for each executable, as the App, the unit tests, and the consumer test do. It copies the DLLs installed by vcpkg (Debug or Release ones, depending on the configuration) after each build, and does nothing with MSVC, whose DLLs are copied by vcpkg, or on Linux.

## Building with CMake Presets from the Command Line

The preset workflow can also be run entirely from a Visual Studio Developer PowerShell:

```powershell
# Use your own vcpkg clone
$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")

# Configure with the Ninja/MSVC x64 Debug preset
cmake --preset ninja-msvc-debug

# Build App and its dependencies
cmake --build --preset ninja-msvc-debug

# Run
out/build/ninja-msvc-debug/bin/App.exe
```

For Release builds, use the `ninja-msvc-release` preset.

## Building Directly with CMake

The project also supports the traditional direct CMake workflow. Run these commands from a Visual Studio Developer PowerShell so that `cl.exe`, the Windows SDK, and the MSVC libraries are available:

```powershell
# Configure a fresh MSVC x64 Release build with Ninja and the dependencies built by vcpkg
cmake -S . -B out/build/manual-ninja-msvc-release -G Ninja `
   -DCMAKE_BUILD_TYPE=Release `
   -DCMAKE_C_COMPILER=cl `
   -DCMAKE_CXX_COMPILER=cl `
   -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
   -DVCPKG_TARGET_TRIPLET=x64-windows

# Build App and its dependencies
cmake --build out/build/manual-ninja-msvc-release --parallel

# Run from the generated bin directory
out\build\manual-ninja-msvc-release\bin\App.exe
```

For a Debug build, replace `Release` with `Debug` and use a separate directory, for example `out/build/manual-ninja-msvc-debug`. With Ninja, `--config Release` is optional because Ninja is a single-configuration generator; `CMAKE_BUILD_TYPE` selects the configuration during configure.

Do not reuse a build directory previously configured with MinGW, Visual Studio, or another generator. Use a new directory or remove the old CMake cache first.

## Command-Line Options and Tests

The App accepts the following command-line options:

- `--scene NAME` (or `--scene=NAME`): load the scene with the given name (`example` by default). The scenes are registered in `app/scenes/Scenes.cpp`: the example scene, and test scenes for reflective and refractive materials and for performance (stress tests).
- `--objects N` (or `--objects=N`): number of objects of the scenes that allow choosing it (the stress test scenes, e.g., `--scene geometric-stress-1 --objects 5000`).
- `--list-scenes`: print the available scenes, with their descriptions, and exit.
- `--frames N` (or `--frames=N`): render `N` frames and exit. The main loop does not wait for user input in this mode, so it can run unattended.
- `-h`, `--help`: print the usage and exit.

In VS Code, the arguments can be set in the `args` of the launch configuration; in Visual Studio, in the debug settings of the `App` target (**Debug** > **Debug and Launch Settings for App**, `"args"`).

With the `ENGINE_BUILD_TESTS` option enabled, the build registers two kinds of tests in CTest:

- **Unit tests** (label `unit`): the `Engine_Unit_Tests` executable, written with [GoogleTest](https://github.com/google/googletest), tests the Core classes that do not require an OpenGL context (e.g., `String_Utils`, `File_System_Utils`, and `Random`), so they run on any machine. GoogleTest is taken from an installed package if one is found (e.g., `libgtest-dev` on Linux), and is otherwise downloaded (pinned version and hash) during the configuration.
- **Smoke tests** (label `smoke`): run `App --frames 120` (the example scene) and a few frames of each test scene (with 10 objects in the stress test scenes), and fail if the App exits with an error code (e.g., the OpenGL 4.5 context cannot be created, or a shader fails to compile) or prints any `[ERROR` message. Another test checks that an unknown scene name is rejected.

The build presets only build the App, so the unit test executable must be requested explicitly:

```bash
# Configure with tests enabled, build, and run the tests (Linux example)
cmake --preset ninja-gcc-debug -DENGINE_BUILD_TESTS=ON
cmake --build --preset ninja-gcc-debug --target App Engine_Unit_Tests
ctest --test-dir out/build/ninja-gcc-debug --output-on-failure

# Run only the unit tests, or only the smoke tests
ctest --test-dir out/build/ninja-gcc-debug -L unit --output-on-failure
ctest --test-dir out/build/ninja-gcc-debug -L smoke --output-on-failure
```

The unit test executable can also be run directly (e.g., `out/build/ninja-gcc-debug/bin/Engine_Unit_Tests`), which accepts GoogleTest's options, such as `--gtest_filter=StringUtilsTest.*`.

On Windows, use the corresponding preset (e.g., `ninja-msvc-debug`). Visual Studio generators are multi-configuration, so they also need the configuration: `ctest --test-dir out/build/vs2026-msvc-debug -C Debug --output-on-failure`. The smoke tests open a window, so they require a display with OpenGL 4.5 support; on a headless Linux machine they can run in a virtual X server with Mesa's software renderer (e.g., `xvfb-run -a ctest --test-dir out/build/ninja-gcc-debug --output-on-failure`).

## Continuous Integration

Every push to `main` and every pull request runs the GitHub Actions workflow in `.github/workflows/ci.yml` (it can also be started manually from the **Actions** tab). Its jobs run in parallel:

- **Format:** checks the formatting of every C++ source file with clang-format 23.1.1, the version used to format the repository (`clang-format --style=file --dry-run --Werror`).
- **Linux:** configures and builds the `ninja-gcc-debug` and `ninja-clang-release` presets with the system packages, and runs the unit tests and the smoke tests in a virtual X server (Xvfb) with Mesa's `llvmpipe` software renderer (OpenGL 4.5).
- **Consumer test (Linux):** builds and runs `tests/consumer`, a standalone project that consumes the Engine with `add_subdirectory()` and links `Engine::Platform`, and checks that the sample App is not built for consumers.
- **Windows:** configures and builds the `ninja-msvc-debug` (MSVC) and `mingw-gcc-vcpkg-release` (MinGW's GCC) presets with the dependencies of `vcpkg.json`, runs the unit tests, and builds the consumer test. vcpkg is checked out at the manifest's `builtin-baseline`, and the built packages are kept in the GitHub Actions cache, so they are only rebuilt when the manifest or the compiler version changes. The smoke tests do not run on Windows, since the GitHub-hosted Windows runners provide no OpenGL 4.5 driver.

The consumer test can also be run locally. It is a separate CMake project, so the compiler and, on Windows, the vcpkg toolchain and the Engine's manifest must be selected explicitly, as the presets do for the Engine. Each compiler needs its own build directory: delete a directory (or configure it with `cmake --fresh`) before reusing it with another compiler.

On Linux:

```bash
cmake -S tests/consumer -B out/build/consumer-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/consumer-gcc
ctest --test-dir out/build/consumer-gcc --output-on-failure
```

On Windows with MSVC and vcpkg, from an ordinary PowerShell with `VCPKG_ROOT` pointing to your vcpkg clone (the Visual Studio generator locates MSVC by itself, so no Developer PowerShell is needed; use `"Visual Studio 17 2022"` for Visual Studio 2022):

```powershell
cmake -S tests/consumer -B out/build/consumer-msvc -G "Visual Studio 18 2026" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_MANIFEST_DIR="$PWD"
cmake --build out/build/consumer-msvc --config Debug
ctest --test-dir out/build/consumer-msvc -C Debug --output-on-failure
```

On Windows with MinGW and vcpkg, from an ordinary PowerShell (with the same environment as the `mingw-gcc-vcpkg-*` presets, including `$env:MINGW_ROOT\bin` in the `PATH`):

```powershell
cmake -S tests/consumer -B out/build/consumer-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic -DVCPKG_MANIFEST_DIR="$PWD"
cmake --build out/build/consumer-mingw
ctest --test-dir out/build/consumer-mingw --output-on-failure
```

Each job has a time limit (10 minutes for formatting, 30 minutes for Linux builds, and 90 minutes for Windows builds, where vcpkg may build the dependencies from source), so a stalled job fails instead of running for hours.

## Release Packages

The `.github/workflows/release.yml` workflow builds the `ninja-msvc-release` preset and packages the App for Windows: when a release is published on GitHub, it attaches `opengl-graphics-engine-<tag>-windows-x64.zip` to it, and when it is started manually (**Actions** > **Release** > **Run workflow**), it uploads the package as an artifact of the run, to check it before a release.

The package is the installation of the `App` component, which contains the App, its DLLs (including the MSVC runtime, so the Visual C++ Redistributable is not required), and the resources, but not the Core and Platform libraries. The same package can be created locally from a Release build:

```powershell
cmake --install out/build/ninja-msvc-release --component App --prefix out/package/opengl-graphics-engine
```

The App is then run from `out/package/opengl-graphics-engine/bin`, and uses the resources of `share/EngineProject/resources`.

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

- **GLFW not found, and no `Running vcpkg install` in the output:** The build directory was configured before without the vcpkg toolchain, and CMake ignores a new toolchain file in an existing build directory. Reconfigure it with `cmake --fresh --preset <preset>` (or **CMake: Delete Cache and Reconfigure** in VS Code).
- **`Could not find toolchain file: /scripts/buildsystems/vcpkg.cmake`:** `VCPKG_ROOT` is not set in the current process. Set it as a user environment variable (see [Dependency Resolution](#dependency-resolution)) and restart VS Code or the terminal.
- **GLFW or Assimp not found on Windows:** Check that `VCPKG_ROOT` points to a vcpkg Git clone (not the vcpkg instance of Visual Studio) that contains the `builtin-baseline` commit of `vcpkg.json`, and that the vcpkg output of the configuration reports no build errors. Then reconfigure with `cmake --fresh --preset <preset>`.
- **Environment value looks correct but CMake uses another path:** The current terminal may have inherited a stale `VCPKG_ROOT`. Run `$env:VCPKG_ROOT = [Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")`, or close and reopen VS Code.
- **GLAD errors:** `core/glad.c` and `external/include/glad/glad.h` are generated with glad 0.1.36 for OpenGL 4.5 Core, without extensions. If they are missing, regenerate them from the [GLAD service](https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D4.5) or with `pip install glad==0.1.36` and `python -m glad --profile=core --api="gl=4.5" --generator=c --spec=gl --extensions="" --out-path <dir>`.
- **Failed to create GLFW window:** The engine requires an OpenGL 4.5 core profile context. Update the GPU drivers, and check the version reported by the GPU (e.g., with the `glxinfo -B` command on Linux, or tools such as GPU Caps Viewer or OpenGL Extensions Viewer on Windows). Remote desktop sessions and some virtual machines only provide older OpenGL versions.
- **DLL missing when running an executable:** With MSVC, vcpkg copies the DLLs beside the executable after each build, and with MinGW, `engine_stage_runtime_dependencies` does; rebuild the target if they are missing. MinGW executables also need the MinGW runtime DLLs (e.g., `libstdc++-6.dll`), so run them with `$env:MINGW_ROOT\bin` in the `PATH`.

For additional help, consult the main [README](../README.md) or raise an issue in the repository (if you have one).
