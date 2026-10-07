# Future Work

Based on the experience gained and the limitations identified, the following improvements are proposed. The planned releases come first, followed by the improvements not yet assigned to a release, ordered by estimated effort and priority.

## Completed in v0.9.0

- **Instanced rendering and frustum culling** (draw call bottleneck, Scenario A).
- **Shader storage buffer for lights** (any number of lights).
- **Cross-platform portability and CI/CD**: Windows (MSVC, MinGW) and Linux (GCC, Clang) builds with GitHub Actions, GoogleTest unit tests, and smoke tests rendered headlessly with Mesa's software renderer.

## Planned Releases

| Version | Content |
| ------- | ------- |
| **v0.10.0** | Deferred shading (G-buffer and lighting pass) and shadow mapping (directional lights first). |
| **v0.11.0** | Normal mapping, Physically Based Rendering (PBR), Screen Space Ambient Occlusion (SSAO), and post-processing (tone mapping, gamma correction). |
| **v0.12.0** | Order-independent transparency, compute shaders, and particles. |
| **v0.13.0** | Editor improvements: data-driven materials, multi-selection, viewport gizmos, and multi-camera support. |
| **v1.0.0** | Stable interfaces, once the Engine has been used by the X-ray simulator. |

The X-ray simulator (`xray-sim`), which consumes the Engine as a Git submodule together with gVirtualXray, starts after v0.10.0, in parallel with the following releases.

## Short-Term (1 - 3 Months)

1. **Optimise Cubemap Updates**  
   - Reduce capture resolution (e.g., 128×128 or 256×256).  
   - Update environment maps only when the object moves, or at a lower frequency (e.g., every N frames).  
   - *Addresses:* Dynamic environment map cost (Scenario C).

2. **Geometry Shaders for Cubemap Arrays**  
   - Render all six faces of a cubemap in a single pass using `gl_Layer` in the geometry shader.  
   - *Addresses:* Cubemap capture overhead.

## Medium-Term (3 - 6 Months)

1. **Full Assimp Integration**  
   - Import animations, bones, morph targets, and all material maps (normal, specular, roughness, etc.).  
   - Implement skinning in the vertex shader.

2. **Automated Rendering Tests**  
   - Compare the rendered images of the test scenes with reference images in the CI (with a tolerance), to detect rendering regressions automatically.

## Long-Term (6 - 12 Months)

1. **Additional Post-Processing**  
    - Anti-aliasing (MSAA/FXAA).  
    - Bloom (Gaussian blur + additive composition).

2. **Additional Lighting Effects**  
    - Shadow mapping for point and spot lights (directional lights come first, in v0.10.0).  
    - Parallax mapping.

3. **Advanced Window Customisation**  
    - Hide the console (`/SUBSYSTEM:WINDOWS`, `FreeConsole()`).  
    - Custom icon (via `.rc` resources).  
    - Fullscreen mode and embedded (child) window modes.

4. **Procedural Generation**  
    - Compute shaders for mesh generation (terrain, noise).

These enhancements would transform the academic prototype into a robust, production-ready engine suitable for games, simulations, and scientific visualisation.
