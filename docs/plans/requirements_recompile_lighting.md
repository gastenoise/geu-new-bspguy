# Requirements Document: Recompile Lighting (RAD) Overhaul

## Introduction

The **Recompile Lighting (RAD)** feature in `revamped-newbspguy` enables mappers and level editors to recalculate lightmaps on compiled GoldSrc/BSP maps using external radiosity compilers (such as `hlrad.exe` from ZHLT or VHLT) without requiring full map recompilation from source `.map` files.

Currently, the feature fails silently, does not execute the compiler when paths contain spaces or relative references, corrupts in-memory face data during sidecar export, loses compiler options across sessions, and provides no error feedback or progress indication when compilers fail.

This document formalizes the requirements to fix, harden, and elevate the Recompile Lighting system into a robust, observable, and fully integrated workflow.

---

## Glossary

- **HLRAD / RAD**: Half-Life Radiosity lighting compilation utility (part of ZHLT, VHLT, or Uncle Mike tools) responsible for calculating direct/indirect lightmaps (`LUMP_LIGHTING`).
- **`.ext` (Face Extents File)**: Intermediate file storing texture bounding box extents (`mins[0..1] maxs[0..1]`) for every face in the BSP.
- **`.wa_` (WAD Sidecar File)**: Temporary WAD archive containing texture headers, dimensions, and palettes needed by HLRAD for surface radiosity calculations.
- **`_nolight.bsp`**: Temporary BSP clone whose lighting lump is cleared prior to feeding it into HLRAD.
- **`LUMP_LIGHTING`**: The BSP lump (index 8) containing raw RGB lightmap sample data.
- **`FindPathInAssets`**: Editor asset resolution engine that resolves relative filenames against map directories, game folders, FGD paths, and configured resource locations.
- **Hot-Reloading**: Unloading the old BSP renderer state and reloading the newly compiled BSP into the editor viewport without restarting the application or losing camera orientation.

---

## Requirements

### Requirement 1: Robust Path Resolution & Process Execution
**User Story:** As an editor user, I want the editor to correctly launch my configured RAD compiler regardless of whether its path is relative, absolute, contains spaces, or is located in an asset directory.

#### Acceptance Criteria
1. THE system SHALL resolve the RAD executable path using `FindPathInAssets(map, g_settings.rad_path, resolved_path)`.
2. THE system SHALL pass the fully resolved path (`resolved_path`) to `Process`, rather than the raw `g_settings.rad_path`.
3. THE system SHALL ensure all executable paths and argument tokens containing whitespace are strictly quoted before invoking OS process creation (`CreateProcessA` on Windows / `execvp` on POSIX).
4. IF the resolved executable file does not exist or is not executable, THE system SHALL abort before modifying any files and display an informative error modal to the user.
5. THE system SHALL capture the process exit code and standard output / error streams for diagnostics.

---

### Requirement 2: Reliable Sidecar Generation & BSP Integrity (`ExportExtFile`)
**User Story:** As a mapper, I want the intermediate files (`.ext`, `.wa_`, `_nolight.bsp`) generated accurately without corrupting the open map currently in memory.

#### Acceptance Criteria
1. THE system SHALL modify only the temporary cloned BSP instance (`tmpBsp->faces[i].nLightmapOffset = -1`) and SHALL NOT mutate the active map's face array (`this->faces`).
2. THE system SHALL clear `LUMP_LIGHTING` on the temporary BSP (`tmpBsp->lumps[LUMP_LIGHTING].clear()`) with header lump offset and length set to `0`.
3. THE system SHALL compute face extents via `GetFaceExtents` for every face and write the extents table to `<map>_nolight.ext`.
4. THE system SHALL extract all textures (embedded and referenced in loaded external WADs) into `<map>_nolight.wa_`.
5. IF texture extraction or file creation fails during `ExportExtFile`, THE system SHALL safely abort compilation and clean up any partial files.

---

### Requirement 3: INI Persistence & Configuration Presets
**User Story:** As a user, I want my compiler options and executable path saved across sessions, and I want quick presets for common lighting quality levels.

#### Acceptance Criteria
1. THE system SHALL persist `hlrad_path` and `hlrad_options` in the `[PATHS]` section of `bspguy.ini`.
2. WHEN `Settings::loadSettings()` is called, THE system SHALL load `hlrad_options` from `bspguy.ini`, defaulting to `"{map_path}"` if absent.
3. WHEN `Settings::saveSettings()` is called, THE system SHALL write `hlrad_options` to `bspguy.ini`.
4. THE Settings dialog (`GuiDialogs.cpp`) SHALL provide preset options buttons:
   - **Fast**: `-fast -nomip {map_path}`
   - **Standard**: `-chart -estimate {map_path}`
   - **High Quality / Extra**: `-extra -bounce 4 -chop 64 {map_path}`
   - **Custom**: Free-form input field.
5. THE Settings dialog SHALL support interactive file picking via `ifd::FileDialog` with automatic path normalization.

---

### Requirement 4: Verification, Error Handling & User Feedback
**User Story:** As a mapper, I want clear feedback during and after compilation so I immediately know if lighting calculation succeeded, failed, or generated warnings.

#### Acceptance Criteria
1. THE system SHALL check both the process exit code (`exitCode == 0`) and verify that `LUMP_LIGHTING` in the resulting BSP contains valid data (`nLength > 0`).
2. IF compilation fails (non-zero exit code or lighting unchanged):
   - THE system SHALL preserve the original `.bsp` file intact.
   - THE system SHALL retain the compiler log (`.log` / `.err`) for inspection.
   - THE system SHALL display an error popup dialog with the failure summary and a button to view the compiler log.
3. WHEN compilation succeeds:
   - THE system SHALL atomically replace the original BSP with the newly lit BSP.
   - THE system SHALL delete all temporary intermediate files (`_nolight.bsp`, `_nolight.ext`, `_nolight.wa_`, `_nolight.log`, `_nolight.err`).
   - THE system SHALL display a success notification in the status bar / log.

---

### Requirement 5: Seamless Viewport Hot-Reloading & Camera Preservation
**User Story:** As a mapper, I want the editor viewport to immediately refresh and display the newly compiled lightmaps at my exact current camera position.

#### Acceptance Criteria
1. WHEN reloading the compiled map, THE system SHALL save the camera origin and Euler angles.
2. THE system SHALL reload the BSP data and re-upload lightmaps to OpenGL textures via `rend->loadLightmaps()`.
3. THE system SHALL restore the saved camera position and orientation seamlessly.
4. THE system SHALL maintain any existing entity or model selection if the corresponding entity IDs remain valid.

---

### Requirement 6: Menu Bar & Action Registry Integration
**User Story:** As a power user, I want to trigger lighting recompilation via the Map menu, shortcut keys, or the Command Palette (`Ctrl+K`).

#### Acceptance Criteria
1. THE `Map` menu SHALL display `Recompile Lighting (RAD)...` with an informative tooltip indicating the configured compiler path.
2. THE system SHALL register an action `map.recompile_lighting` in `ActionRegistryInit.cpp`.
3. The action SHALL be searchable and executable from the Command Palette (`Ctrl+K`).
