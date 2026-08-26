# Design Document: Recompile Lighting (RAD) Overhaul

## Overview

This document specifies the technical design and architectural improvements required to repair and upgrade the **Recompile Lighting (RAD)** subsystem in `revamped-newbspguy`. 

The design establishes reliable executable discovery, quoted process execution, non-destructive sidecar file generation, INI configuration persistence with quality presets, rigorous validation of output lighting lumps, robust error reporting via ImGui modals, and seamless in-place viewport reload preserving camera and selection state.

---

## System Architecture

### Component Map

| Component ID | Component Name | Type | Responsibility | Interfaces With |
|---|---|---|---|---|
| `COMP-1` | `Process` Utility | Core / OS Subsystem | Process launching, quoting, stdout/stderr capture, exit code polling | Windows Win32 API / POSIX `fork`/`execvp` |
| `COMP-2` | `Bsp::ExportExtFile` | BSP Data Export | Generates `.ext` face extents, `.wa_` textures, and `_nolight.bsp` without touching active memory | `Bsp`, `Wad`, `GetFaceExtents` |
| `COMP-3` | `Settings` Persistence | Configuration | Loads and saves `hlrad_path` and `hlrad_options` to `bspguy.ini` | `bspguy.ini`, `GuiDialogs` |
| `COMP-4` | `GuiDialogs` (RAD Settings) | UI / Dialogs | Provides executable picker and preset buttons (`Fast`, `Standard`, `Extra`, `Custom`) | `Settings`, `ifd::FileDialog` |
| `COMP-5` | `RadCompileController` / `GuiMenuBar` | Workflow Orchestration | Validates inputs, triggers export, invokes compiler process, verifies output, triggers reload or error modal | `BspRenderer`, `Renderer`, `Process`, `ImGui` |
| `COMP-6` | `BspRenderer` Hot-Reload | Viewport Rendering | Replaces BSP data, reloads lightmaps, and restores camera parameters | OpenGL, `Bsp` |

---

### High-Level Architecture Diagram

```
+-------------------------------------------------------------------------------+
|                             revamped-newbspguy                                |
|                                                                               |
|  +---------------------+        +--------------------+                        |
|  | Menu / Action / UI  | -----> | RadCompileWorkflow |                        |
|  +---------------------+        +---------+----------+                        |
|                                           |                                   |
|               +---------------------------+---------------------------+       |
|               |                           |                           |       |
|               v                           v                           v       |
|      [1. Asset Resolver]         [2. Sidecar Export]         [3. Settings]    |
|       FindPathInAssets           Bsp::ExportExtFile          Load/Save INI    |
|               |                           |                           |       |
|               +---------------------------+---------------------------+       |
|                                           |                                   |
|                                           v                                   |
|                            +----------------------------+                     |
|                            |  Process (Quoted Win32)    |                     |
|                            +--------------+-------------+                     |
+-------------------------------------------|-----------------------------------+
                                            |
                                            v (spawns child process)
                           +--------------------------------+
                           |           hlrad.exe            |
                           |  Reads:  .bsp, .ext, .wa_      |
                           |  Writes: .bsp (LUMP_LIGHTING)  |
                           |          .log, .err            |
                           +----------------+---------------+
                                            |
                                            v (exit code + file verification)
+-------------------------------------------|-----------------------------------+
|                                           v                                   |
|                            +----------------------------+                     |
|                            | Output & Lump Verification |                     |
|                            +--------------+-------------+                     |
|                                           |                                   |
|                  +------------------------+------------------------+          |
|                  | (Success: nLength > 0)                          | (Failed) |
|                  v                                                 v          |
|      +-----------------------+                         +------------------+   |
|      | Atomic Replace &      |                         | Keep BSP intact, |   |
|      | BspRenderer Hot-Reload|                         | Show Error Modal |   |
|      +-----------------------+                         +------------------+   |
+-------------------------------------------------------------------------------+
```

---

## Data Flow Specifications

### 1. Pre-Compilation & Validation Flow
```
User Action ("Recompile Lighting")
   │
   ├──► 1. Check if g_settings.rad_path is populated.
   │       IF empty ──► Show error dialog: "No RAD compiler configured. Set in Settings."
   │
   ├──► 2. Resolve executable path:
   │       FindPathInAssets(map, g_settings.rad_path, resolvedExePath)
   │       IF !fileExists(resolvedExePath) ──► Show error modal: "RAD executable not found at: <path>".
   │
   └──► 3. Save camera state & current map edits:
           map->update_ent_lump();
           map->update_lump_pointers();
           map->write(map->bsp_path);
```

### 2. Sidecar Generation Flow (`ExportExtFile`)
```
ExportExtFile(old_bsp_path, out_bsp_path)
   │
   ├──► 1. Derive base path prefix: <mapname>_nolight
   │
   ├──► 2. Create and write <mapname>_nolight.ext:
   │       - Write faceCount.
   │       - For each face [0..faceCount-1]:
   │           Calculate GetFaceExtents(i, mins, maxs)
   │           Write "mins[0] mins[1] maxs[0] maxs[1]\n".
   │
   ├──► 3. Create <mapname>_nolight.bsp:
   │       - Instantiate temporary Bsp: tmpBsp = new Bsp(old_bsp_path).
   │       - Clear lighting lump: tmpBsp->lumps[LUMP_LIGHTING].clear().
   │       - Set header: tmpBsp->bsp_header.lump[LUMP_LIGHTING].nOffset = 0, nLength = 0.
   │       - Set tmpBsp->faces[i].nLightmapOffset = -1 (FIX: NEVER modify active map faces).
   │       - tmpBsp->update_lump_pointers();
   │       - tmpBsp->write(out_bsp_path);
   │       - delete tmpBsp;
   │
   └──► 4. Create <mapname>_nolight.wa_:
           - Gather embedded textures and external WAD textures loaded in renderer.
           - Write textures to temporary WAD archive.
```

### 3. Process Execution & Command Line Formatting Flow
```
Process Execution
   │
   ├──► 1. Parse g_settings.rad_options template (e.g. "-chart -estimate {map_path}").
   │       Replace "{map_path}" with out_bsp_path.
   │
   ├──► 2. Format Windows Command Line:
   │       - Program: quoteIfNecessary(resolvedExePath)
   │       - Arguments: Ensure proper token quoting if parameters contain paths with spaces.
   │
   ├──► 3. Launch Process:
   │       - Execute CreateProcessA with STARTF_USESHOWWINDOW or console redirection.
   │       - WaitForSingleObject(hProcess, INFINITE).
   │       - Retrieve exitCode via GetExitCodeProcess.
```

### 4. Output Verification & Hot-Reload Flow
```
Compilation Evaluation
   │
   ├──► Check 1: Did process exit with code 0?
   ├──► Check 2: Does out_bsp_path exist and has fileSize > 0?
   ├──► Check 3: Load header of out_bsp_path -> Does LUMP_LIGHTING have nLength > 0?
   │
   ├──► IF ALL CHECKS PASS (Success):
   │       1. Backup or remove old_bsp_path.
   │       2. Atomic rename/copy: copyFile(out_bsp_path, old_bsp_path).
   │       3. Reload map in BspRenderer:
   │          - Save camera origin & angles.
   │          - Reload BSP lumps and call rend->loadLightmaps().
   │          - Restore camera.
   │       4. Clean up temporary files (.wa_, .ext, .log, .err, _nolight.bsp).
   │       5. Log success message.
   │
   └──► IF ANY CHECK FAILS (Failure):
           1. Leave original BSP untouched.
           2. Retain .log / .err sidecars for user debugging.
           3. Display Modal Error Dialog with:
              - Process exit code.
              - Error summary.
              - Button to open the generated compilation log file.
```

---

## Component Details & Fix Specifications

### 1. `Process` Quoting & Win32 Execution Fix
In `src/util/util.h` and `src/util/util.cpp`:
- Enable automatic executable quoting:
  ```cpp
  std::string Process::getCommandlineString()
  {
      std::stringstream cmdline;
      if (_program.find(' ') != std::string::npos && _program.front() != '\"')
          cmdline << '\"' << _program << '\"';
      else
          cmdline << _program;

      for (const auto& arg : _arguments)
      {
          cmdline << " " << arg;
      }
      return cmdline.str();
  }
  ```
- Ensure `CreateProcessA` receives a properly formatted command line buffer and handles spaces in application paths safely.

### 2. `Bsp::ExportExtFile` Pointer Correction
In `src/bsp/Bsp.cpp`:
- Correct lines 14332-14336:
  ```cpp
  // FIX: Modify tmpBsp->faces, NOT this->faces!
  for (int i = 0; i < tmpBsp->faceCount; i++)
  {
      tmpBsp->faces[i].nLightmapOffset = -1;
  }
  ```
- Verify that `this->faces` on the active map remains completely untouched during the export process.

### 3. `Settings.cpp` Persistence for `hlrad_options`
In `src/editor/Settings.cpp`:
- In `Settings::loadSettings()`:
  ```cpp
  g_settings.rad_options = settings_ini->Get<std::string>("PATHS", "hlrad_options", "\"{map_path}\"");
  ```
- In `Settings::saveSettings()`:
  ```cpp
  iniData << "hlrad_path=" << g_settings.rad_path << "\n";
  iniData << "hlrad_options=" << g_settings.rad_options << "\n";
  ```
- In `Settings::loadDefaultSettings()`:
  ```cpp
  rad_options = "\"{map_path}\"";
  ```

### 4. Configuration Presets in `GuiDialogs.cpp`
In `src/editor/gui/GuiDialogs.cpp` (Settings -> Paths tab):
- Provide preset buttons:
  - `[Fast]`: sets `g_settings.rad_options = "-fast -nomip \"{map_path}\"";`
  - `[Standard]`: sets `g_settings.rad_options = "-chart -estimate \"{map_path}\"";`
  - `[Extra / High]`: sets `g_settings.rad_options = "-extra -bounce 4 -chop 64 \"{map_path}\"";`
  - `[Reset]`: resets to `"\"{map_path}\""`.

### 5. Error Dialog & Modal Feedback
In `src/editor/gui/GuiDialogs.cpp`:
- Implement a modal dialog `DrawRadCompileErrorModal()` when compilation fails, displaying:
  - Header: `"RAD Compilation Failed"`
  - Description: Exit code, failure reason (e.g. executable not found, compiler exited with error).
  - Actions: `"Open Log File"`, `"Open Settings"`, `"Close"`.

---

## Testing & Verification Strategy

1. **Unit / Path Resolution Tests**:
   - Verify `FindPathInAssets` with relative paths (`hlrad.exe`), absolute paths (`C:\Tools\hlrad.exe`), and paths with spaces (`C:\GoldSrc Tools\hlrad.exe`).
2. **Sidecar Verification**:
   - Verify `.ext` file structure matches HLRAD expectations (`faceCount` on line 1, 4 integers per face).
   - Verify `.wa_` archive is a valid WAD3 file containing required texture headers.
   - Verify `_nolight.bsp` contains 0-length `LUMP_LIGHTING` while retaining all geometry lumps.
3. **End-to-End Compile Tests**:
   - Run compilation on a sample map using a standard HLRAD / VHLT compiler.
   - Verify compiler execution, log file generation, successful return code, and lighting lump population.
   - Verify viewport hot-reloads lighting immediately with no crashes and exact camera preservation.
