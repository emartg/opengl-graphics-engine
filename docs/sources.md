# Sources and Attribution

This file credits the third-party libraries and the resources that are part of the project (and are meant
to stay in it): who made each one, where it comes from, and under which license. The temporary assets
(placeholders) are listed at the end and will be credited if they are kept in the final version. The
shaders and materials in `resources/shaders` and `assets/materials` are the project's own work and fall
under its MIT License (see `LICENSE`).

## How to Use This File

- Add an entry when a library or a resource becomes part of the project: its name and version, author,
  source link, and license.
- A temporary asset goes under "Placeholders". When it is kept, move it up and complete its entry; when it
  is replaced, remove it.
- Checked on 2026-10-11: the licenses were read from the files in the repository or from the official
  repository of each project.

## Libraries

| Library | Version | Used for | Author and source | License | How it is provided |
|---|---|---|---|---|---|
| [GLFW](https://www.glfw.org) | 3.5.1 | Window, input, OpenGL context | Marcus Geelnard and the GLFW contributors | zlib License | vcpkg or system package |
| [GLM](https://github.com/g-truc/glm) | 1.0.3 | Vector and matrix mathematics | G-Truc Creation | MIT License (also offered under the Happy Bunny License; either may be used) | vcpkg or system package |
| [Assimp](https://github.com/assimp/assimp) | 6.0.5 | 3D model import | The assimp team | BSD 3-Clause | vcpkg or system package |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.12.0 | JSON descriptors of shaders and materials | Niels Lohmann | MIT License | vcpkg or system package |
| [GoogleTest](https://github.com/google/googletest) | 1.18.0 | Unit tests (only with `ENGINE_BUILD_TESTS`) | Google | BSD 3-Clause | installed package, or a pinned download |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.91.6 | Immediate-mode GUI | Omar Cornut and contributors | MIT License (its bundled `stb_rect_pack` 1.01 and `stb_truetype` 1.26 are public domain, as their headers state) | Bundled in `external/include/ImGui` |
| [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) | 0.6.8 | File dialogs | Stephane Cuillerdier (Aiekick) | MIT License (`external/include/ImGuiFileDialog/LICENSE`); it bundles its own `stb_image` 2.29 (public domain) | Bundled in `external/include/ImGuiFileDialog` |
| [stb_image](https://github.com/nothings/stb) | 2.30 | Image loading | Sean Barrett and contributors | MIT License or public domain, at the user's choice | Bundled in `external/include/stb_image.h` |
| [glad](https://github.com/Dav1dde/glad) | 0.1.36 (generator) | OpenGL 4.5 core loader | David Herberth (generator, copyright 2013-2022) | The generator is MIT (see open items for the generated loader) | Generated code in `core/glad.c` and `external/include/glad` |
| Khronos `khrplatform.h` | n/a | Platform types of the OpenGL headers | The Khronos Group Inc., copyright 2008-2018 | MIT-style license in the file header | Bundled in `external/include/KHR` |

OpenGL itself (version 4.5 core) is an API of the Khronos Group; the engine uses the implementation of the
graphics driver.

## Resources

| Path | What | Author and source | License | Status |
|---|---|---|---|---|
| `resources/fonts/RobotoMono-*.ttf` (Bold, BoldItalic, Italic, Light, LightItalic, Medium, MediumItalic, Regular, SemiBoldItalic) | Font of the GUI | Roboto Mono, designed by Christian Robertson for Google ([googlefonts/RobotoMono](https://github.com/googlefonts/robotomono)) | Apache License 2.0, copyright 2015 The Roboto Mono Project Authors | Identified from the font family's published information; the repository has no license file for it |

## Placeholders

Temporary assets for the sample scenes. They are credited as far as the credit is known; credit the ones
that stay, and replace those whose source or license cannot be established. The release packages include
`resources/`, so they ship with them for now.

| Path | What | Author, source, and license known so far |
|---|---|---|
| `assets/skyboxes/faces/` (`fatbursparken`, `meadow`, `medborgarplatsen`, `mountain_path`, `saint_lazarus_church`, `swedish_royal_castle`, `tantolunden`, `union_square`, `union_square_INVALID`) | Cube map skyboxes | Emil Persson, aka Humus (http://www.humus.name), [CC BY 3.0](http://creativecommons.org/licenses/by/3.0/), stated in the `readme.txt` of each folder (the license requires this credit) |
| `assets/skyboxes/hdr/` (`canary_wharf`, `golden_gate_hills`, `potsdamer_platz`, `shanghai_bund`, `tiergarten`) | Environment maps | [Poly Haven](https://polyhaven.com) ([CC0](https://polyhaven.com/license)): Canary Wharf by Andreas Mischok; Golden Gate Hills by Dimitrios Savva (photography) and Jarod Guest (processing); Potsdamer Platz, Shanghai Bund, and Tiergarten by Greg Zaal. Identified by file name |
| `assets/skyboxes/hdr/puresky_4k.hdr` | Environment map | Unknown (probably a renamed Poly Haven "Pure Sky" asset) |
| `assets/models/` (teapot in OBJ, FBX, and glTF; construction helmet; traffic cone) | Sample models with PBR textures | Unknown (the files only show Blender and Maya exports) |

## Open Items

1. **glad:** confirm the license of the generated loader (`core/glad.c`, `external/include/glad`) against
   the generator's license page, and say so here.
2. **License texts of bundled code:** only ImGuiFileDialog ships its license text. Consider adding the
   licenses of Dear ImGui, stb, and glad next to their folders, and the Apache License 2.0 text next to the
   Roboto Mono fonts (the license asks that recipients receive a copy).
3. **Placeholders:** when the final assets are chosen, complete or replace the entries above.
