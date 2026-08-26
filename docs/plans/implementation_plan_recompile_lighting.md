# Implementation Plan: Recompile Lighting (RAD) Overhaul

## Project Boundaries

- **Must-Have (In Scope)**:
  - Fix `Process` Win32 command-line formatting and path quoting for executables/arguments containing whitespace.
  - Fix `FindPathInAssets` usage in `GuiMenuBar.cpp` to pass the resolved path to `Process`.
  - Fix memory corruption bug in `Bsp::ExportExtFile` where `this->faces` was mutated instead of `tmpBsp->faces`.
  - Persist `hlrad_options` in `bspguy.ini` across sessions via `Settings.cpp`.
  - Add compiler option presets (`Fast`, `Standard`, `Extra`, `Reset`) to Settings dialog in `GuiDialogs.cpp`.
  - Implement robust output verification (`exitCode == 0`, lighting lump size check `nLength > 0`).
  - Implement user feedback and failure diagnostics modal dialog in `GuiDialogs.cpp`.
  - Ensure seamless viewport hot-reloading with camera position/angle preservation.
  - Register `map.recompile_lighting` action in `ActionRegistryInit.cpp` for Command Palette support.

- **Out of Scope (Deferred)**:
  - Rewriting HLRAD from scratch in C++.
  - Full multithreaded asynchronous background job queue (modal wait with status is sufficient for v1).
  - Editing individual lightmap luxels manually in the 3D viewport.

---

## Phase Breakdown

- [x] 1. Core Fixes: Process Quoting & ExportExtFile Pointer Fix
  - [x] 1.1 Fix `Process` command-line generation and executable quoting in `src/util/util.cpp` and `src/util/util.h`
    - Ensure `_program` with spaces is wrapped in double quotes
    - Validate proper argument token handling for Win32 `CreateProcessA`
    - _Requirements: REQ-1.3_
  - [x] 1.2 Fix `Bsp::ExportExtFile` memory corruption in `src/bsp/Bsp.cpp`
    - Change `faces[i].nLightmapOffset = -1` to `tmpBsp->faces[i].nLightmapOffset = -1`
    - Ensure active map memory (`this->faces`) remains untouched
    - _Requirements: REQ-2.1, REQ-2.2_
  - [x] 1.3 Fix executable path resolution in `src/editor/gui/GuiMenuBar.cpp`
    - Pass resolved `path` to `new Process(path)` instead of raw `g_settings.rad_path`
    - _Requirements: REQ-1.1, REQ-1.2_

- [x] 2. Settings Persistence & Configuration Presets
  - [x] 2.1 Implement `hlrad_options` INI load/save in `src/editor/Settings.cpp`
    - Load `hlrad_options` in `Settings::loadSettings()` with default `"{map_path}"`
    - Save `hlrad_options` in `Settings::saveSettings()` under `[PATHS]`
    - Add to `Settings::loadDefaultSettings()`
    - _Requirements: REQ-3.1, REQ-3.2, REQ-3.3_
  - [x] 2.2 Enhance Settings Dialog in `src/editor/gui/GuiDialogs.cpp`
    - Add preset buttons for `Fast`, `Standard`, `Extra`, and `Reset`
    - Support browse button with path normalization
    - _Requirements: REQ-3.4, REQ-3.5_

- [x] 3. Robust Verification, Error Diagnostics & Hot-Reloading
  - [x] 3.1 Implement output and lighting lump verification in `src/editor/gui/GuiMenuBar.cpp`
    - Check process exit code (`exitCode == 0`)
    - Inspect compiled BSP header to verify `LUMP_LIGHTING` has non-zero size
    - _Requirements: REQ-4.1_
  - [x] 3.2 Add compilation error modal / log viewer in `src/editor/gui/GuiDialogs.cpp`
    - Display error popup when compiler fails or executable is missing
    - Provide button to inspect generated `.log` or `.err` file
    - _Requirements: REQ-4.2_
  - [x] 3.3 Ensure seamless map reload and camera preservation
    - Atomically copy new BSP over old BSP
    - Reload lightmaps and restore camera origin and angles
    - Clean up temporary files on success
    - _Requirements: REQ-4.3, REQ-5.1, REQ-5.2, REQ-5.3_

- [x] 4. Action Registry & Command Palette Integration
  - [x] 4.1 Register `map.recompile_lighting` action in `src/editor/actions/ActionRegistryInit.cpp`
    - Define action name, label, category, and callback
    - _Requirements: REQ-6.2, REQ-6.3_
  - [x] 4.2 Update Menu Bar entry and tooltips in `src/editor/gui/GuiMenuBar.cpp`
    - Update menu item text to `Recompile Lighting (RAD)...` with descriptive tooltip
    - _Requirements: REQ-6.1_

- [x] 5. Verification & Testing
  - [x] 5.1 Build and compile the project in Release mode
    - Ensure 0 compilation errors and 0 warnings on modified files
  - [x] 5.2 Test end-to-end compilation with a sample BSP
    - Test with paths containing spaces
    - Test with invalid compiler path (verifying error modal)
    - Test with valid HLRAD (verifying lightmaps are updated in viewport)
