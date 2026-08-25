# Requirements Document: End-to-End Skybox Rendering

## Introduction

The goal of this feature is to implement complete, in-engine-accurate 3D skybox rendering within `revamped-newbspguy`. Currently, surfaces textured with sky materials (`sky`, `SKY`, `skycull`) are rendered with a flat 2D dummy placeholder texture mapped across local polygon coordinates. 

In GoldSrc and related engines, sky surfaces act as transparent portals into an infinite background cube rendered from 6 distinct directional image files (`up`, `dn`, `ft`, `bk`, `lf`, `rt`). The skybox background remains stationary with respect to camera translation while rotating seamlessly with camera viewing angles.

This document formalizes the requirements for skybox asset discovery, texture loading, GLSL shader projection, rendering integration, UI controls, and fallback mechanics.

---

## Glossary

- **Skybox**: A 6-sided cubic environment map providing a background at optical infinity.
- **`skyname`**: Keyvalue defined in the map's `worldspawn` entity (entity 0) specifying the base filename prefix of the 6 skybox texture images.
- **GoldSrc Sky Sides**: The 6 standard directional suffixes used in GoldSrc/Half-Life:
  - `up`: Top / +Z
  - `dn`: Down / -Z
  - `ft`: Front / +X
  - `bk`: Back / -X
  - `lf`: Left / +Y
  - `rt`: Right / -Y
- **View-Ray Projection**: A shader technique where the skybox is sampled using the normalized direction vector from the camera origin to the world-space vertex/fragment, projecting the sky seamlessly across arbitrary BSP sky polygons without translation parallax.
- **`FindPathInAssets`**: Internal asset resolver searching through map directories, game directories, FGD paths, and resource paths.
- **Cubemap (`GL_TEXTURE_CUBE_MAP`)**: OpenGL texture target containing 6 square 2D faces forming a seamless cube.

---

## Requirements

### Requirement 1: Skybox Asset Discovery & Multi-Tier Resolution
**User Story:** As a mapper, I want the editor to automatically locate skybox image files according to standard game directories and custom paths so that maps display their intended skies without manual asset copying.

#### Acceptance Criteria
1. THE system SHALL read the `skyname` property from the map's `worldspawn` entity (`map->ents[0]`).
2. IF `skyname` is empty or missing, THE system SHALL default to `"desert"` for GoldSrc maps.
3. THE system SHALL search for the 6 sky images (`<skyname>up`, `<skyname>dn`, `<skyname>ft`, `<skyname>bk`, `<skyname>lf`, `<skyname>rt`) across supported image formats (`.tga`, `.png`, `.bmp`, `.jpg`).
4. THE search resolution SHALL follow this precedence hierarchy:
   1. User-configured Skybox Directory (if set in Settings).
   2. Standard Game Directory path: `<gamedir>/gfx/env/` and `<gamedir>/env/`.
   3. Asset search paths via `FindPathInAssets(map, "gfx/env/" + filename, ...)`.
   4. Default fallback sky (`"desert"`) using tiers 1-3.
   5. Classic 2D flat placeholder (`sky.png`).
5. WHEN all 6 sides are found, THE system SHALL load them into a single skybox resource.

---

### Requirement 2: 6-Sided Texture Loading & GPU Resource Management
**User Story:** As an editor user, I want high-quality skybox rendering without visible seams or edge artifacts at face boundaries.

#### Acceptance Criteria
1. THE system SHALL decode `.tga`, `.png`, `.bmp`, and `.jpg` image formats using `stb_image` / `lodepng`.
2. THE system SHALL upload the 6 faces to an OpenGL Cubemap (`GL_TEXTURE_CUBE_MAP`) or 6-face texture sampler array.
3. THE texture parameters SHALL configure `GL_TEXTURE_WRAP_S`, `GL_TEXTURE_WRAP_T`, and `GL_TEXTURE_WRAP_R` to `GL_CLAMP_TO_EDGE` to eliminate border seams.
4. THE texture filtering SHALL use `GL_LINEAR` for magnification and minification.
5. Skybox texture loading and GPU uploading SHALL occur asynchronously alongside standard texture streaming (`texturesFuture`) without blocking the main UI thread.

---

### Requirement 3: View-Ray Shader Projection & Skybox Rendering
**User Story:** As a mapper navigating the 3D viewport, I want the skybox to render fixed at infinity through all sky surfaces exactly as it does in Half-Life.

#### Acceptance Criteria
1. THE system SHALL implement a dedicated Skybox Shader Program (`skyShader`) computing view-ray directions `normalize(worldPos - cameraOrigin)`.
2. THE skybox projection SHALL only rotate with camera pitch/yaw angles and SHALL NOT translate with camera movement.
3. WHEN `RENDER_SKYBOX` is enabled and 6-sided sky textures are loaded, all faces with `isSky` (textures starting with `sky`, `SKY`, `skycull`) SHALL render using the Skybox Shader.
4. IF `RENDER_SKYBOX` is disabled OR 6-sided textures cannot be resolved (including `"desert"` fallback), THE system SHALL render sky faces using the classic planar 2D shader with `skyTex_rgba`.
5. THE skybox rendering SHALL function correctly in normal viewport rendering, screenshot capture FBOs, and overview generation.

---

### Requirement 4: Settings, Configuration & INI Persistence
**User Story:** As a user, I want to configure the skybox search directory and toggle skybox rendering on or off, with preferences saved across sessions.

#### Acceptance Criteria
1. THE system SHALL declare `RENDER_SKYBOX = 1 << 18` within `enum RenderFlags` in `Settings.h`.
2. THE system SHALL add `std::string skybox_dir` to `Settings` with default empty string `""`.
3. THE system SHALL persist `render_skybox` and `skybox_dir` in `settings.ini` via `Settings::loadSettings()` and `Settings::saveSettings()`.
4. THE Settings dialog (`GuiDialogs.cpp` -> Render tab) SHALL provide:
   - A checkbox for `"Render Skybox"`.
   - A text input field for `"Skybox Directory"` with a `"Browse..."` file/folder dialog button.

---

### Requirement 5: Menu Bar, Action Registry & Shortcut Integration
**User Story:** As a power user, I want fast access to toggle the skybox through the menu bar, command palette, and keyboard shortcuts.

#### Acceptance Criteria
1. THE system SHALL add a `"Skybox"` toggle item under the `View` menu in `GuiMenuBar.cpp`.
2. THE system SHALL register an action `"view.skybox_toggle"` in `ActionRegistryInit.cpp` with description `"Toggle 3D Skybox Rendering"`.
3. THE Command Palette (`Ctrl+K`) SHALL list and execute the `"view.skybox_toggle"` action.

---

### Requirement 6: Performance & Stability
**User Story:** As a user editing large maps, I want skybox rendering to maintain stable 60+ FPS without memory leaks or crashes during map switching.

#### Acceptance Criteria
1. THE system SHALL release all allocated GPU textures and memory when a map is unloaded or reloaded.
2. Frame rendering overhead for skybox surfaces SHALL NOT exceed 1.0 millisecond per frame on standard GPU hardware.
3. Rapid map switching, model editing, or undo/redo operations SHALL NOT corrupt or invalidate skybox texture state.
