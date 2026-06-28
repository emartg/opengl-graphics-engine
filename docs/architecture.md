
# Architecture Guide

This document provides an overview of the engine's internal design.

## Modules and Separation

The project is split into two top-level modules:

1. **`core` (static library)** – Platform-agnostic rendering engine. It contains:
   - The scene graph and node system.
   - Render pipeline logic.
   - Resource management (textures, shaders, buffers).
   - Mathematical utilities and geometry primitives.
   - Dependencies: OpenGL, GLAD, GLM, stb_image, Assimp.

2. **`app` (executable)** – Platform-specific application layer. It contains:
   - Window creation and context management (GLFW).
   - Input event handling (callbacks).
   - The ImGui editor interface.
   - Dependencies: GLFW, ImGui.

This strict separation means the `core` can be linked into other applications (e.g., a simulation or scientific visualiser) without pulling in GLFW or ImGui.

## Render Pipeline (5 Phases)

The engine follows a structured, multi-pass pipeline every frame. The sequence is designed to ensure correctness (depth, blending, environment capture) and performance.

### Phase 1: Input & Frame Setup
- Waits for events using `glfwWaitEvents()` (blocks when idle) or `glfwWaitEventsTimeout(1/60)` (throttled active rendering).
- Polls I/O events.
- Builds the ImGui interface state.
- Computes delta time.

### Phase 2: Pre-Render Computations
- Ensures the off-screen framebuffer (FBO) exists and matches the window size.
- Converts HDR skybox images to cubemaps if needed.
- Updates dynamic environment maps for reflective/refractive models (captures six faces per object).
- Processes any pending colour-picking queries.

### Phase 3: Scene Setup & Uniforms
- Binds the off-screen FBO and clears colour/depth/stencil buffers.
- Injects view and projection matrices into shaders.
- Injects light data (arrays of uniforms for point, spot, directional lights).
- Traverses the scene graph, flattens it into a linear list, and splits objects into opaque and transparent lists. Transparent objects are sorted back-to-front.

### Phase 4: Geometry Rendering Passes
- **Opaque Pass** – Renders all opaque nodes (depth write enabled).
- **Skybox Pass** – Renders the environment cubemap.
- **Transparent Pass** – Renders translucent nodes (depth write disabled, depth test enabled, blending active).
- **Gizmos Pass** – Renders light gizmos and directional light arrows (wireframe mode, culling disabled).

### Phase 5: Presentation
- Composites the off-screen FBO to the default framebuffer (allows debug overlays and post-processing).
- Swaps window buffers using `glfwSwapBuffers()`.

## Key Classes

- **`Core`** – Singleton orchestrator. Initialises subsystems, manages the main loop, and delegates responsibilities.
- **`Renderer`** – Abstract interface for rendering. `GLFW_Renderer` (in `app`) is the concrete GLFW implementation.
- **`Node`** – Base class for all scene entities. Implements the **Composite pattern** (parent-child hierarchy). Contains transform logic (position, rotation, scale) and virtual methods (`load()`, `draw()`, `deallocate_resources()`).
- **`Render_Pass`** – Encapsulates off-screen framebuffer management (FBO, attachments, resolution). Used for colour picking and environment map captures.
- **Managers** – Dedicated services:
  - `Scene_Manager` – holds the root node.
  - `Node_Manager` – stores and retrieves nodes by ID.
  - `Selection_Manager` – handles colour-picking and outlining.
  - `Input_Manager` – translates raw input to camera/UI actions.
- **`Light`** – Abstract base for directional, point, and spot lights. Each light owns a visual gizmo (wireframe shape) that is rendered in the gizmo pass.

For UML class diagrams, refer to **Figures 5–9** in the thesis document, which detail the package layout, class hierarchies, and subsystem relationships.