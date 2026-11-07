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

The other images of the folder (the five UML diagrams, the two rendering pipelines, the three performance charts, and the development chronogram) were made before the sources were kept, and have no `.mmd` yet. They are listed in [pending_sources.txt](pending_sources.txt), and each one leaves the list when its source is written.

## Rendering

The images are made by the Mermaid command-line tool, **`@mermaid-js/mermaid-cli` version 11.17.0** (Mermaid 11.17), which needs Node.js and a Chromium-based browser (the tool downloads one the first time it runs). One command renders every diagram:

```
python docs/diagrams/diagrams.py render
```

It runs this command for each `.mmd` (the version is written in the script, `MERMAID_CLI_VERSION`):

```
npx --yes @mermaid-js/mermaid-cli@11.17.0 -i docs/diagrams/<subfolder>/<name>.mmd -o docs/diagrams/<subfolder>/<name>.png -b white -s 2
```

The environment variable `DIAGRAMS_PUPPETEER_CONFIG` names a Puppeteer configuration file (to choose another browser or to pass flags), and `DIAGRAMS_MMDC` names an installed `mmdc` program to use instead of `npx`. The fonts and the colors are in the source of each `.mmd` (the `config:` block of its front matter and its `classDef` lines), and the position of every box is written in it, so that the source alone produces the image. The size of the text depends on the fonts of the browser, so two renders of the same source can differ slightly.

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
