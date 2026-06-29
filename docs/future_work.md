# Future Work

Based on the experience gained and the limitations identified, the following improvements are proposed. They are ordered by estimated effort and priority.

## Short-Term (1 - 3 Months)

1. **Instanced Rendering + Frustum Culling**  
   - Group objects with identical meshes and materials to reduce draw calls drastically.  
   - Implement frustum culling to skip rendering objects outside the camera's view.  
   - *Addresses:* Draw call bottleneck (Scenario A).

2. **Order-Independent Transparency (OIT)**  
   - Replace `O(n log n)` sorting with per-pixel linked lists (or depth peeling) for correct and efficient transparent blending.  
   - *Addresses:* Blending overhead (Scenario B).

3. **Optimise Cubemap Updates**  
   - Reduce capture resolution (e.g., 128×128 or 256×256).  
   - Update environment maps only when the object moves, or at a lower frequency (e.g., every N frames).  
   - *Addresses:* Dynamic environment map cost (Scenario C).

4. **Geometry Shaders for Cubemap Arrays**  
   - Render all six faces of a cubemap in a single pass using `gl_Layer` in the geometry shader.  
   - *Addresses:* Cubemap capture overhead.

## Medium-Term (3 - 6 Months)

1. **Data-Driven Material System**  
   - Define materials, shaders, and textures via JSON/YAML files loaded at runtime.  
   - Allow shader swapping and parameter changes without recompilation.

2. **Full Assimp Integration**  
   - Import animations, bones, morph targets, and all material maps (normal, specular, roughness, etc.).  
   - Implement skinning in the vertex shader.

3. **Viewport Manipulation Gizmos**  
   - Translate, rotate, and scale widgets in the 3D viewport using raycasting.

4. **Multi-Selection**  
   - Support selecting multiple non-sibling nodes via `Ctrl`/`Shift` modifiers, allowing batch transformations.

5. **Multi-Camera Support**  
   - Manage multiple cameras, switch between them, or display several viewports simultaneously.

## Long-Term (6 - 12 Months)

1. **Shader Storage Buffer Objects (SSBOs) for Lights**  
    - Enable thousands of dynamic lights in the scene without per-object uniform limits.

2. **Post-Processing Pipeline**  
    - Anti-aliasing (MSAA/FXAA).  
    - High Dynamic Range (HDR) with tone mapping.  
    - Bloom (Gaussian blur + additive composition).  
    - Gamma correction.  
    - Deferred shading (G-buffer + lighting pass).

3. **Advanced Window Customisation**  
    - Hide the console (`/SUBSYSTEM:WINDOWS`, `FreeConsole()`).  
    - Custom icon (via `.rc` resources).  
    - Fullscreen mode and embedded (child) window modes.

4. **Cross-Platform Portability + CI/CD**  
    - Test on Linux and macOS.  
    - Set up GitHub Actions workflows for automated builds.  
    - Integrate Google Test for unit testing.  
    - Use OSMesa or EGL for headless rendering tests.

5. **Advanced Lighting & Effects**  
    - Shadow mapping (spot, point, directional).  
    - Normal mapping and parallax mapping.  
    - Screen Space Ambient Occlusion (SSAO).  
    - Physically Based Rendering (PBR - Cook-Torrance).

6. **Procedural Generation and Particles**  
    - Compute shaders for particle simulation and mesh generation (terrain, noise).  
    - Geometry shaders for billboard rendering.

These enhancements would transform the academic prototype into a robust, production-ready engine suitable for games, simulations, and scientific visualisation.
