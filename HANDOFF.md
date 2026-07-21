# Handoff

## Repository state

- Canonical repository: `https://github.com/Ding-Ding-Projects/BambuStudio.git`
- Branch: `master`
- Last pushed commit at the start of this handoff update: `63778e0e8` (`Add isolated Git-backed project history core`)
- `origin/master` matched that commit before the remaining native integration work was committed.
- The working tree contains the native Material integration, project-history lifecycle and dialog,
  translations, tests, and hosted-test wiring described below. Those changes still need their final
  review, authoritative build, smoke pass, commits, and push.
- Preserve the user's existing change to
  `src/slic3r/GUI/DeviceWeb/device_page/src/routeTree.gen.ts`. It is unrelated to this effort and must
  not be included in the Material/history commits.

The most recent pushed implementation sequence is:

- `122ac853d` — fix NanoSVG integration in Windows Release builds;
- `e962e099e` — apply Material roles to production workspaces;
- `c39270ca8` — move Prepare actions into the Material bottom bar; and
- `63778e0e8` — add the isolated Git-backed project-history core.

Replace the provisional values above after the final pushes:

- Final tested source commit: **PENDING**
- Final documentation commit: **PENDING**
- Final `master` / `origin/master` equality and worktree audit: **PENDING**

## Native Material implementation

This effort modifies the real native wxWidgets/OpenGL application. The separate `ui-md3` application
remains an interactive design reference; its images are not evidence that the native application
matches the design.

The current native source includes:

- semantic Material light/dark color roles shared by native widgets, with brand-green, Preview-purple,
  and Device-teal contextual schemes;
- a Material top bar with the Bambu Studio identity, File/Edit/View/Objects/Help menus, responsive
  sizing, and a Settings action that opens the real Preferences UI;
- contextual Prepare/Preview/Device navigation state;
- a responsive Prepare sidebar for printer identity, bed selection, synchronization, and one-column
  filament rows;
- a left-side vertical transform/gizmo rail and top-centered scene commands in Prepare;
- a Material Prepare bottom bar with plate selection, add-plate, estimates, Helio, Slice, and Print
  actions wired to the existing application commands;
- a contextual Preview legend/dock, primary view-mode chips, timeline treatment, and localized
  supporting controls; and
- full-width Temperature, Print Options, AMS, and Move cards in the native Device status panel while
  retaining the official networking-plugin gate.

The remaining code review is focused on narrow-window/localization behavior, toolbar highlight
ownership, plate-state refresh, responsive Preview/sequence coexistence, and final lifecycle capture
boundaries. Do not describe Material parity as verified until the final installed executable has been
smoked and its native captures have been visually reviewed.

## App-local project history

The project-history repository is deliberately separate from both the source checkout and the user's
project directory. Production constructs `ProjectHistoryManager` with `Slic3r::data_dir()` and stores
bare repositories below:

```text
<Bambu Studio data directory>/project_history/v1/<SHA-256 project identity>
```

Each Git commit contains a complete immutable `.3mf` snapshot blob. The project path is normalized and
hashed for repository identity, so Bambu Studio never creates a `.git` directory beside a project and
never commits project data to `Ding-Ding-Projects/BambuStudio`.

Lifecycle behavior implemented in the current source is:

- discrete project edits, undo/redo operations, and completed manual saves schedule ordered snapshots;
- snapshots are materialized into private staging files before a serialized worker commits them, and
  byte-identical consecutive snapshots do not create duplicate commits;
- transient capture/commit failures retain pending recovery data where possible, retry with a delay,
  and notify the user instead of silently reporting success;
- shutdown flushes pending capture work and drains submitted worker operations;
- **Save As** forks the complete ancestry to the new path identity and then commits the completed new
  save; an existing unrelated destination history causes migration to fail closed;
- **File → Version history** lists full commit IDs and restores a selected snapshot into the open
  session through a temporary archive; it does not directly overwrite the saved project file; and
- restore first preserves the current state, uses a rollback snapshot on load failure, retains the
  original project identity, and records the successful restored state as a new revision.

Important limits to communicate to users:

- History is local-only. It is not a cloud service, remote Git backup, or collaboration mechanism.
- There is no retention, quota, or pruning policy yet. Complete `.3mf` revisions can consume
  substantial disk space and historical Git objects may retain content removed in later revisions.
- The feature cannot guarantee persistence through an unrecoverable disk-full, permission, filesystem,
  or hardware failure. It warns and retries recoverable failures, but conventional backups remain
  necessary.
- Save As has explicit ancestry migration. Moving or renaming a project outside Bambu Studio can produce
  a different path identity and therefore a different history.
- A restored revision changes the in-memory session. The user's saved project is changed only by a
  later explicit save.

The focused history-core suite previously passed all seven cases for isolated storage, duplicate
suppression, validation, ordered commits, restore safety, identity migration, collision handling, and
shutdown draining. That was not the final integrated binary: rerun the suite after lifecycle changes
and prove multiple discrete UI edits produce distinct reachable snapshots before calling the feature
verified.

## Build and verification state

The dependency superbuild and an earlier Release baseline are available locally under
`deps/build-local` and `install_local`. A targeted compile of the current native integration was still
running when this documentation update began. It is not final evidence.

The final verifier must record all of the following from the same tested source state:

- Release configure/build/install command and result: **PENDING**
- `project_history_tests` result and case count: **PENDING**
- `libnest2d_tests` result and case count: **PENDING**
- `language_mode_tests` result and case count: **PENDING**
- `scripts/ci/Test-WindowsRelease.ps1` result and reported native/DeviceWeb/legacy counts: **PENDING**
- Installed executable path and isolated `--datadir`: **PENDING**
- Native desktop smoke results for Prepare, Preview, Device/plugin gate, dark/light themes, resize,
  localization, slice, project history, restore, and Save As: **PENDING**
- Reachable history commit count and content proof after several separate UI edits: **PENDING**
- Reviewed native screenshot paths: **PENDING**

Use the available low-level desktop/computer-control integration for the installed-app smoke. Window
capture alone may omit the child WGL surface, so README evidence must come from a compositor/full-display
capture and be visually inspected. Do not substitute `ui-md3` reference images or synthetic/mock
screenshots for native evidence.

## Hosted CI and release state

The Windows workflow now requests `project_history_tests` alongside `libnest2d_tests` and
`language_mode_tests`. The latest known run for the pushed history-core commit was:

- `https://github.com/Ding-Ding-Projects/BambuStudio/actions/runs/29753284085` — status must be
  rechecked; it was still in progress when last observed.

The final implementation push will create a newer run, so the run above must not be presented as final
Material/history evidence. Record:

- Final Actions run ID and URL: **PENDING**
- Successful Windows job names and test summaries: **PENDING**
- Installer/SBOM/checksum artifact names and retention: **PENDING**
- Release tag, latest/non-latest decision, and publication state: **PENDING**
- Installer SHA-256, CycloneDX component count, and commit binding: **PENDING**
- `gh attestation verify` result, if release artifacts are published: **PENDING**
- Repository/release immutable state and final branch/stash audit: **PENDING**

Checksums and GitHub attestations are not Authenticode signatures. A trusted Windows signing identity
and securely configured signing provider remain external work. Native Cantonese coverage also remains
a curated preview requiring independent human review for safety-critical, destructive, privacy,
account, recovery, and networking text.

No Postman collection is applicable because this work exposes no HTTP API.
