# Diagrams

Each diagram is kept twice, side by side, in a subfolder named after its topic:

```
docs/diagrams/<subfolder>/<name>.mmd   the Mermaid source (the file to edit)
docs/diagrams/<subfolder>/<name>.png   the rendered image (generated from the source)
```

The images are regenerated and reviewed whenever a release is about to come out, so that every `.png` matches its `.mmd`.

## Index

| Diagram | Source | Image | What it shows |
|---|---|---|---|
| The dependencies and packages | [dependencies/dependencies_and_packages.mmd](dependencies/dependencies_and_packages.mmd) | [dependencies/dependencies_and_packages.png](dependencies/dependencies_and_packages.png) | The three CMake targets (`Engine::Core`, `Engine::Platform`, and `App`) stacked in the middle, with the libraries that each one links on both sides of it (Core's in green, Platform's in orange); every arrow reads "links to" (the dotted one, to `nlohmann_json`, is private); each library says where it comes from (vcpkg or system, a system library, or bundled source) |
| The architecture overview (the top of the UML set) | [uml/architecture_overview.mmd](uml/architecture_overview.mmd) | [uml/architecture_overview.png](uml/architecture_overview.png) | A UML class diagram of the three modules: the blocks have stereotypes that start with their module (App, Platform, or Core), `Application` and `Core` show their operations, and the arrows are UML (composition, aggregation, generalization, realization, dependency) with a word on each; the blocks that group classes (the subsystems) are detailed by the four diagrams below |
| The managers and libraries (detail of the overview) | [uml/managers_subsystem.mmd](uml/managers_subsystem.mmd) | [uml/managers_subsystem.png](uml/managers_subsystem.png) | The box "Managers, libraries": `Core` and the four managers (scene, node, selection, input) and the two libraries (shader, material) that it owns, with the members of each one and the classes that each one holds |
| The renderer, the GUI, and the assets (detail of the overview) | [uml/rendering_and_gui_subsystem.mmd](uml/rendering_and_gui_subsystem.mmd) | [uml/rendering_and_gui_subsystem.png](uml/rendering_and_gui_subsystem.png) | The boxes `Renderer`, `GLFW_Renderer`, `GUI`, `Assets_Window`, and "Assets": the renderer and its implementation, the GUI layer, the render passes, and the asset registry and selection that the Assets window owns |
| The scene entities (detail of the overview) | [uml/scene_entities_subsystem.mmd](uml/scene_entities_subsystem.mmd) | [uml/scene_entities_subsystem.png](uml/scene_entities_subsystem.png) | The box "Scene entities": `Node` and the classes that derive from it (models, lights, camera, texture, and shader), and the meshes, the shared geometry, and the material that a node refers to |
| The utilities and the geometry data (detail of the overview) | [uml/utilities_and_geometry_data.mmd](uml/utilities_and_geometry_data.mmd) | [uml/utilities_and_geometry_data.png](uml/utilities_and_geometry_data.png) | The block "Utilities and Geometry Data": ten UML class blocks (utilities, geometry classes, and the header data), each with its attributes, its operations (static ones underlined), and a last compartment with the classes that use it, and the two dependencies between them as dotted arrows |

The other images of the folder (the two rendering pipelines, the three performance charts, and the development chronogram) were made before the sources were kept, and have no `.mmd` yet. They are listed in [pending_sources.txt](pending_sources.txt), and each one leaves the list when its source is written.

**The UML set is a hierarchy.** `uml/architecture_overview` is the top: it shows the modules and how they relate, and each of its boxes that groups classes has a detailed diagram: "Managers, libraries" is `uml/managers_subsystem`; `Renderer`, `GLFW_Renderer`, `GUI`, `Assets_Window`, and "Assets" are `uml/rendering_and_gui_subsystem`; "Scene entities" is `uml/scene_entities_subsystem`; and "Utilities and Geometry Data" is `uml/utilities_and_geometry_data`. Each `.mmd` of the set says in its first lines which module it shows and which block of the overview it details.

## Rendering

The images are made by the Mermaid command-line tool, **`@mermaid-js/mermaid-cli` version 11.17.0** (Mermaid 11.17), which needs Node.js and a Chromium-based browser (the tool downloads one the first time it runs). One command renders every diagram:

```
python docs/diagrams/diagrams.py render
```

It runs this command for each `.mmd` (the version is written in the script, `MERMAID_CLI_VERSION`):

```
npx --yes @mermaid-js/mermaid-cli@11.17.0 -i docs/diagrams/<subfolder>/<name>.mmd -o docs/diagrams/<subfolder>/<name>.png -b white -s 2
```

The environment variable `DIAGRAMS_PUPPETEER_CONFIG` names a Puppeteer configuration file (to choose another browser or to pass flags), and `DIAGRAMS_MMDC` names an installed `mmdc` program to use instead of `npx`. The fonts and the colors are in the source of each `.mmd` (the `config:` block of its front matter and its `classDef` lines), and the position of every box is written in it, so that the source alone produces the image. The size of the text depends on the fonts of the browser, so two renders of the same source can differ in size and layout, not only by a few pixels (a render on another machine was 2400 by 1350 pixels where the committed one is 1568 by 882).

**A render is committed only when it is more accurate or clearer than the image it replaces, and together with the `.mmd` that produced it.** Look at every image before committing it, and check it against the code. A render of a source that did not change, which only looks different, is discarded (`git restore` the image). A source that does not reproduce the committed image is first brought to its quality (a title, a theme, a legend that says what each color is), or the committed image is kept.

## Checking

```
python docs/diagrams/diagrams.py check
```

stops with an error when a `.png` has no `.mmd` and is not listed in `pending_sources.txt`, when a `.mmd` has no `.png` (the image was not rendered), when a listed image already has its `.mmd` or does not exist (the line is removed), when an image is not a PNG file, or when a source is empty. It needs only Python and runs in the workflow `diagrams.yml` on every pull request and push to `main`, in a few seconds. It does not check that an image is up to date with its source: that is what rendering and reviewing before a release is for.

## Adding a diagram

1. Write `docs/diagrams/<subfolder>/<name>.mmd` (names in lowercase with underscores) from the code, not from memory, and add its line to the index above.
2. Render it with `python docs/diagrams/diagrams.py render` and look at the image. Commit only the image of a diagram whose source changed and whose render is more accurate or clearer (see the note on rendering).
3. If the image was in `pending_sources.txt`, remove its line.
4. Commit the `.mmd` and the `.png` together, and show the image in [the architecture guide](../architecture.md) when it belongs there.

Choose the Mermaid diagram type for the layout that the diagram needs: a `flowchart` places the boxes by itself, and a `block-beta` diagram places every box in a grid that the source writes (the dependency diagram uses it: the targets are stacked in the middle and each one has its libraries beside it, never more than one row away, so that no arrow crosses a box and the image stays compact). Arrows go straight between the centers of two boxes, so keep a box off the line between two others. A block diagram has no `linkStyle`, so the kind of an arrow is written as the text on it.

**Class diagrams** (`classDiagram`) are laid out with the ELK engine (`layout: elk` in the front matter), which draws orthogonal arrows around the boxes instead of through them; `class: useMaxWidth: false` keeps the natural size. The image is the diagram scaled to the width of the browser window of the tool (about 784 px, times the scale 2), so a diagram whose natural width is larger than that gets smaller text: read the natural size from the `viewBox` of the SVG (`mmdc -i name.mmd -o name.svg`) and keep it under about 1600 px, which gives text of about 1% of the width of the image, the size of the images that the UML set replaced. When the layout of a class diagram grows too tall or too wide, remove the members that do not explain the diagram, give a box at least one member so that it has no empty compartment, and try `direction LR` against `direction TB`. A catalogue of classes that are not related to each other (like `utilities_and_geometry_data`) cannot be laid out as a class diagram, because the automatic layouts put unrelated classes in one row or one column: draw it as a block diagram whose blocks imitate the class compartments (see below).

**UML diagrams use UML class blocks and keep one style** (decided 2026-11-09): every diagram of `uml/` draws its classes, interfaces, and subsystems as class blocks (the stereotype and the name, then the attributes and the operations, with the arrowheads of UML), with the fonts, the colors, and the stereotypes of the others, and plain boxes (`block-beta`, flowcharts) are for the diagrams that are not UML (the dependencies, the pipelines, and the charts). Legibility and correctness come first, and the size of the image that is replaced is a reference, never a reason to drop the notation or the information. When a class diagram cannot be laid out legibly by the tool, as for a catalogue of unrelated classes, a block diagram can draw the blocks with the compartments of a class: each block is a label of HTML with `<br/>`, `<hr/>` between the compartments, `<div align='left'>` for the members, `&laquo;` and `&raquo;` for the stereotype (the source stays in ASCII), `<u>` for static members, and the fill `#fff4dd` and the border `#eedebb` of the class blocks of Mermaid; its relations that are real are drawn as dotted arrows with a free cell between the blocks they join.
