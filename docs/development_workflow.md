# Development Workflow

This document describes how changes are made, reviewed, and released in this repository.

## Branches and Pull Requests

`main` always contains working code: changes are not pushed to it directly, but merged through pull requests, after the CI workflow passes on them.

1. Create a branch from an up-to-date `main`, with a short descriptive name (e.g., `feature/deferred-shading`, `fix/mesh-buffers`, `docs/v0.9.0-status`):

   ```powershell
   git switch main
   git pull
   git switch -c feature/deferred-shading
   ```

2. Commit the changes on the branch (one or more commits, following the commit message conventions below), and push it:

   ```powershell
   git push -u origin feature/deferred-shading
   ```

3. Open a pull request on GitHub (the push output prints the link). The CI workflow runs on it; if it fails, push the fixes to the same branch.
4. Once the CI passes, merge the pull request with **Rebase and merge**, which replays its commits on top of `main` (keeping a linear history and the original commit messages, although the commits get new hashes), and delete the branch.
5. Update the local copy: `git switch main`, `git pull`, and `git branch -d <branch>`.

## Commit Messages

- The subject is a short imperative sentence in Title Case (e.g., `Cull the Nodes Outside the Camera's View Frustum`).
- The body starts with an introductory paragraph, followed by the `Summary:`, `Changes:` (grouped by area), `Notes:` (optional), `Validation:`, and `Conclusion:` sections.
- Each commit is a self-contained change that builds and passes the tests.

## Changelog

Every pull request with a notable change adds a line to the `[Unreleased]` section of the [Changelog](../CHANGELOG.md), under `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, or `Security`.

## Versions

The project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html): before v1.0.0, minor versions (`0.x.0`) add features and may include breaking changes, and patch versions (`0.x.y`) only include fixes. Versions before v1.0.0 are published as pre-releases.

## Releases

1. In a release pull request (e.g., `release/v0.9.0`):
   - Rename the `[Unreleased]` section of the changelog to the version and date (e.g., `## [0.9.0] - 2026-10-08`), add a new empty `[Unreleased]` section above it, and update the comparison links at the end of the file.
   - Make sure that the `project()` version in the top-level `CMakeLists.txt` matches the new version.
   - Update the status of the README and the [Limitations](limitations.md) and [Future Work](future_work.md) documents if needed.
2. Merge the pull request once the CI passes.
3. On GitHub, draft a new release: create the tag (`v0.9.0`) on `main` when publishing, use the title `Release v0.9.0 - <Highlights>`, write the release notes (Overview, sections per area, Upgrade Notes when needed, Summary, and the full changelog link), and mark it as a pre-release before v1.0.0.
4. Publishing the release starts the Release workflow, which attaches the Windows package (`opengl-graphics-engine-<tag>-windows-x64.zip`) to it a few minutes later. Before the release, the package can be checked by starting the workflow manually (**Actions** > **Release** > **Run workflow**), which uploads it as an artifact of the run.
