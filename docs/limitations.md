
# Limitations and Known Issues

As of version **v0.8.0** (the latest beta release), the engine is functional but has several known constraints. These are grouped into architectural and performance limitations.

## Architectural Limitations

| Limitation | Description | Affected Areas |
| ---------- | ----------- | -------------- |
| **Static Shader/Material Assignment** | Shaders are assigned by node type via hard-coded logic in `Renderer.cpp`. No data-driven system exists to change materials at runtime without recompiling. | Flexibility, extensibility. |
| **Limited Assimp Import** | Assimp is used only for static geometry extraction. Animations, bones, morph targets, and complex material maps (normals, specular, etc.) are not imported. Some embedded textures fail to load. | Model import quality. |
| **No Shadows** | Lights are evaluated without shadow mapping. Shadow maps, point shadows, and cascaded maps are not implemented. | Visual realism. |
| **Low Light Count** | The number of lights per scene is limited to a small static array (no SSBOs). | Scalability. |
| **Rotation Instability** | *Y-axis rotation ghosting* and *rotation axis coupling* appear due to quaternion-Euler conversions, causing abrupt jumps when rotating in certain orientations. | Usability. |
| **Residual Flicker** | In dense reflective/refractive scenes, minor flicker can persist on environment maps, despite ping-pong buffering mitigation. | Visual quality. |
| **No Automated Tests** | No unit or integration tests. Validation is done manually via visual inspection and ad-hoc benchmarks. | QA, regression prevention. |
| **Windows Dependency** | The code uses Windows-specific libraries (`windows.h`) for certain optimisations. Linux and macOS are untested. | Portability. |

## Performance Limitations

| Limitation | Scenario | Impact |
| ---------- | -------- | ------ |
| **Draw Call Bottleneck** | Opaque objects (Scenario A). Each object issues individual uniforms (`glUniformMatrix4fv`). No instancing or frustum culling. | Drops sharply >1000 objects (~76 FPS at 1000, ~16 FPS at 5000). |
| **Blending Overhead** | Transparent objects (Scenario B). Back-to-front sorting adds `O(n log n)` CPU cost, while blending increases fragment operations (overdraw). | >500 transparent objects becomes impractical (~19 FPS at 500, ~9 FPS at 1000). |
| **Dynamic Environment Maps** | Reflective/refractive objects (Scenario C). Each object renders the entire scene six times per frame for its cubemap. | >10 reflective objects tanks performance (~99 FPS at 10, ~7 FPS at 50). |

The following charts allow for a visualization of these disparities based on the retrieved data:

<p></p>
<div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"> <img src="diagrams/performance/avg_fps_per_n_cubes.png" alt="avg_fps_per_n_cubes" title="Avg. FPS per number of cubes" style="height:400px; width:auto;" /> </div>

<p></p>
<div style="display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:20px;"> <img src="diagrams/performance/avg_processing_time_per_n_cubes_AB.png" alt="avg_processing_time_per_n_cubes_AB" title="Avg. processing time per number of cubes (A vs. B)" style="height:350px; width:auto;" /> <img src="diagrams/performance/avg_processing_time_per_n_cubes_C.png" alt="avg_processing_time_per_n_cubes_C" title="Avg. processing time per number of cubes (C)" style="height:350px; width:auto;" /> </div>

## Missing Features (Unfulfilled Objectives)

The following objectives from the thesis plan were not implemented due to time and health constraints:

- Multiple selection of non-sibling nodes.
- Direct manipulation gizmos (translate/rotate/scale widgets in the viewport).
- Post-processing (anti-aliasing, gamma correction, HDR, bloom, deferred shading).
- Multi-camera support.
- Procedural generation and particle systems (geometry/compute shaders).
- Advanced window adaptation (fullscreen mode, embedded mode, custom icon, console hiding).

For planned improvements to address these, see the [Future Work](future_work.md) document.
