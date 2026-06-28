
# Build Instructions

This document provides detailed steps to compile and run the OpenGL Graphics Engine.

## Prerequisites

- **Operating System:** Windows (tested on Windows 11). Linux/macOS are not officially tested but CMake configuration is prepared for them.
- **Compiler:** Visual Studio 2022 (with C++20 support) or any C++20 compiler.
- **CMake:** ≥ 3.20.
- **Git:** To clone the repository (optional if you download the source).

## Dependency Resolution

The engine uses CMake's `find_package` with custom `Find*.cmake` scripts located in `/cmake/modules` for the following libraries:

- GLFW
- GLM
- Assimp

For OpenGL (GLAD), `stb_image`, and ImGui, the required files are either generated (GLAD) or included directly as headers (stb_image, ImGui backends) in the source tree.

The build will attempt to locate these dependencies on your system. If not found, you may need to install them manually (e.g., via vcpkg, Conan, or direct download) and set the appropriate `CMAKE_PREFIX_PATH`.

## Building with Visual Studio (Ninja Generator)

1. **Clone or download** the source code.

2. **Open the root folder** in Visual Studio. Visual Studio will automatically detect the CMake project.

3. **Configure CMake**:
   - VS will run CMake configuration automatically.
   - You can manually trigger it via `Project > Configure Cache`.
   - The default generator is Ninja.

4. **Build**:
   - Build the entire solution: `Build > Build All`.
   - Alternatively, build only the `App` target.

5. **Run**:
   - Set `App.exe` as the startup item (if not already).
   - Press `F5` (Debug) or `Ctrl+F5` (Run without debugging).

## Building from Command Line

If you prefer the command line:

```bash
# Configure with Ninja
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
build/bin/App.exe
```

For Debug builds, replace `Release` with `Debug`.

## Clean Build

To clean all generated files:

```bash
cmake --build build --target clean
```

To delete the entire build directory (nuclear option):

```bash
rm -rf build/   # Linux/macOS
rmdir /s build  # Windows (cmd)
```

## Troubleshooting

- **Missing dependencies:** If CMake fails to find a library, ensure it is installed and its path is added to `CMAKE_PREFIX_PATH`. For example: `-DCMAKE_PREFIX_PATH="C:/path/to/libs"`.
- **GLAD errors:** The `glad.c` and `glad.h` are generated in `core/third_party/glad/`. If they are missing, regenerate them from the [GLAD service](https://glad.dav1d.de/) using OpenGL 4.2 Core.
- **Assimp DLL missing:** If running the executable fails with a missing `assimp-vc143-mtd.dll`, copy it from your Assimp installation into the same directory as `App.exe`, or add its folder to your `PATH`.

For additional help, consult the main `README.md` or raise an issue in the repository (if you have one).