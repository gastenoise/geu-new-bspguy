# Requirements Document: GoldSrc-Accurate Decal Rendering and Interaction

## Introduction

The goal of this feature is to implement complete, in-engine-accurate rendering and interactive selection of decal entities (`infodecal`) within `revamped-newbspguy`.

In GoldSrc and related engines, static map decals are defined as point entities (`infodecal`) carrying a texture name key (e.g., `"{target1"`, `"{blood1"`, `"{hand1"`). In the engine:
1. Decal textures are 8-bit indexed images stored in WAD files (such as `decals.wad`).
2. Decal textures are grayscale intensity maps where white is transparent and darker tones represent opacity.
3. The actual rendered color of the decal in the 3D world is defined by the **last color in the texture's 256-color palette** (`palette[255]`).
4. Decals are projected onto the nearest BSP surface at their native pixel dimensions (1:1 world unit to texel ratio, unscaled) with alpha transparency and without scaling.
5. Decal geometry is clipped against the host face edges (Sutherland-Hodgman convex polygon clipping) so decals do not overhang into empty space.
6. Decal lighting mode can be toggled between Fullbright / Unlit (standard editor view for maximum clarity) and Lightmap-Modulated (matching in-game lighting conditions).
7. In the editor, when decal rendering is active, interacting with the decal in the 3D viewport (clicking on the decal's surface area) selects the entity, and selecting the entity highlights the decal on the world surface rather than rendering a generic bounding cube.

---

## Glossary

- **`infodecal`**: Point entity in GoldSrc BSP maps specifying a static decal placed at a given `origin` with a specific `texture` key.
- **Decal Palette Coloring**: The GoldSrc rendering rule where the RGB color of the decal is derived from `palette[255]` of the 256-color texture palette, while the pixel grayscale values define transparency.
- **Decal Alpha Map**: Conversion of the 8-bit grayscale pixel brightness $L$ ($0 \dots 255$) into alpha $\text{Alpha} = 255 - L$, where $L = 255$ (pure white) is fully transparent ($\text{Alpha} = 0$) and $L = 0$ (black) is fully opaque ($\text{Alpha} = 255$).
- **Surface Texture Alignment**: The $(\vec{vS}, \vec{vT})$ basis vectors from `texinfo` that determine the horizontal and vertical orientation of the projected decal on the BSP face.
- **Decal Polygon Clipping**: Sutherland-Hodgman clipping of the decal quad against host face edges so decal geometry conforms precisely to face boundaries.
- **Decal Lighting Mode**: Setting allowing decals to be rendered either Fullbright (unlit) or modulated by surface lightmaps.
- **`RENDER_DECALS`**: Render flag bit toggling decal rendering in the 3D viewport.

---

## Requirements

### Requirement 1: Decal Texture Decoding & Palette Color Extraction
**User Story:** As a mapper, I want decal textures to be loaded and colored exactly as they appear in the GoldSrc engine so that visual appearance matches gameplay.

#### Acceptance Criteria
1. THE system SHALL look up the texture specified by the `texture` key of each `infodecal` in the loaded WADs (including `decals.wad` and map-referenced WADs).
2. THE system SHALL decode the 8-bit indexed miptex data and its 256-color palette (`COLOR3 palette[256]`).
3. THE system SHALL use the color at index 255 (`palette[255]`) as the base RGB color for all pixels of the decal.
4. FOR EACH pixel, THE system SHALL compute the alpha component from the grayscale intensity $L = \text{palette}[\text{index}].r$:
   - IF $L == 255$, THEN $\text{Alpha} = 0$ (fully transparent).
   - ELSE, $\text{Alpha} = 255 - L$.
5. THE system SHALL upload the resulting RGBA data to an OpenGL texture with wrap mode `GL_CLAMP_TO_EDGE` and texture type `TYPE_DECAL`.
6. THE system SHALL cache uploaded decal textures so identical decal textures across multiple entities share GPU resources.

---

### Requirement 2: Surface Search & Planar Decal Projection with Clipping
**User Story:** As an editor user, I want decals to automatically find and project onto the nearest BSP face, clipping cleanly to face boundaries so they don't float in empty air.

#### Acceptance Criteria
1. FOR EACH `infodecal` entity, THE system SHALL find the closest candidate BSP face (from worldspawn and brush models) within a proximity threshold ($\le 8.0$ units from the face plane).
2. THE system SHALL project the entity's `origin` point onto the face plane to establish the decal center $P = \vec{\text{origin}} - \vec{N}(\vec{N} \cdot \vec{\text{origin}} - D)$.
3. THE system SHALL determine the decal orientation axes $(\vec{S}_{\text{dir}}, \vec{T}_{\text{dir}})$ from the target face's `texinfo` $(\vec{vS}, \vec{vT})$ vectors (or canonical plane axes via `TextureAxisFromPlane` if `texinfo` vectors are degenerate).
4. THE system SHALL generate a 3D planar quad centered at $P$ with width $W$ and height $H$ equal to the texture's native dimensions in world units (1 texel = 1 unit).
5. THE system SHALL clip the decal quad against each bounding edge plane of the host face polygon using convex polygon clipping (Sutherland-Hodgman) so decal geometry does not extend beyond the host face.

---

### Requirement 3: 3D Viewport Decal Rendering & Configurable Lighting
**User Story:** As a user, I want decals to render smoothly on walls without Z-fighting, with clear selection feedback and configurable lighting (Fullbright vs Lightmap Modulated).

#### Acceptance Criteria
1. WHEN `RENDER_DECALS` is enabled, THE system SHALL render projected decals on their host surfaces instead of the default point entity cube.
2. THE system SHALL render decals using alpha blending (`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`) and polygon depth offset (`glPolygonOffset(-1.0f, -1.0f)`) to prevent Z-fighting.
3. THE system SHALL support a configurable Decal Lighting Mode:
   - **Fullbright / Unlit Mode (Default):** Decals are rendered with full texture brightness regardless of darkness/lightmaps.
   - **Lightmapped / Modulated Mode:** Decals sample the underlying surface lighting/lightmap intensity.
4. WHEN an `infodecal` entity is selected, THE system SHALL render the decal with selection highlighting (yellow wireframe outline around the decal polygon and coordinate axes).
5. IF a decal cannot be projected (missing texture or no adjacent surface found), THE system SHALL fall back to rendering the standard point entity cube so the entity remains visible and editable.
6. WHEN `RENDER_DECALS` is disabled, THE system SHALL render `infodecal` entities using the standard point entity cube.

---

### Requirement 4: Raycast Picking & Bi-directional Selection
**User Story:** As a user, I want to select an `infodecal` by clicking anywhere on its projected decal in the 3D viewport, and see it highlighted when selected in the entity list.

#### Acceptance Criteria
1. WHEN `RENDER_DECALS` is enabled, clicking on the projected decal surface in the 3D viewport SHALL select the corresponding `infodecal` entity.
2. Raycast hit testing SHALL intersect the camera ray with the decal polygon/quad plane and verify the hit point lies within the decal's bounded polygon.
3. Clicking on an area of the wall outside the decal boundaries SHALL pass the raycast through to the underlying BSP surface/entity.
4. Selecting an `infodecal` in the Entity Report or Keyvalue Editor SHALL highlight the decal on the world surface in the 3D viewport.

---

### Requirement 5: Settings, Menu Bar, Action Registry & Localization
**User Story:** As a user, I want to toggle decal rendering and lighting mode from the View menu, Settings window, and command palette, with my choices saved across sessions.

#### Acceptance Criteria
1. THE system SHALL define `RENDER_DECALS = 1 << 19` in `enum RenderFlags` (`Settings.h`).
2. THE system SHALL persist `render_decals` and `decal_lighting_mode` in `settings.ini` via `Settings::loadSettings()` and `Settings::saveSettings()`.
3. THE system SHALL add a `"Decals"` checkbox under the `View` menu (`GuiMenuBar.cpp`).
4. THE system SHALL add `"Render Decals"` and `"Decal Lighting (Fullbright / Lightmapped)"` controls in `Settings -> 3D View` (`GuiDialogs.cpp`).
5. THE system SHALL register actions `"view.decals_toggle"` and `"view.decals_lighting_toggle"` in `ActionRegistryInit.cpp` and support execution via the Command Palette (`Ctrl+K`).
6. THE system SHALL add localized string entries in `src/util/lang_defs.h` and `resources/languages/language*.ini`.

---

### Requirement 6: Dynamic Updates & Transformation Handling
**User Story:** As a mapper, when I move an `infodecal` or edit its `texture` key, I want the decal rendering and projection to update immediately.

#### Acceptance Criteria
1. WHEN an `infodecal` entity is moved, rotated, or has its keyvalues edited, THE system SHALL refresh its projected geometry in `BspRenderer::refreshEnt()`.
2. WHEN undoing or redoing an entity transformation/edit, THE decal projection and rendering SHALL update synchronously.
