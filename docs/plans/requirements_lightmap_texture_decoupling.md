# Requirements Document: Decoupled Lightmap Preservation on Face Texture Modifications

## Introduction

In GoldSrc / Quake BSP30 maps, lightmaps and surface textures are geometrically coupled to a single data structure (`BSPTEXTUREINFO`). Whenever a mapper selects faces and edits texture properties (scale, shift/position, rotation, or texture dimensions), the engine recalculates the lightmap sample grid dimensions and offsets.

In the current implementation of `revamped-newbspguy`, modifying texture properties breaks, shifts, or corrupts face lightmaps both in the real-time viewport and in the saved BSP lighting lump. This document defines the formal requirements for decoupling texture UV modifications from lightmap coordinates, preserving world-space lighting alignment through geometrically exact resampling, and eliminating cumulative visual degradation.

---

## Glossary

- **`BSPTEXTUREINFO` (`texinfo`)**: BSP lump structure storing 3D texture projection vectors `vS`, `vT` and 1D offsets `shiftS`, `shiftT`.
- **`BSPFACE32`**: BSP structure representing a polygon face, referencing `iTextureInfo`, lighting styles `nStyles[4]`, and `nLightmapOffset`.
- **Lightmap Grid**: A 2D discrete grid of lighting samples spaced at 16 texture units ($s, t$).
- **Surface Extents**: The bounding box $[s_{\min}, s_{\max}] \times [t_{\min}, t_{\max}]$ in texture coordinates converted to discrete lightmap dimensions $W = \lfloor s_{\max}/16 \rfloor - \lfloor s_{\min}/16 \rfloor + 1$.
- **World-Space Resampling**: An inverse-matrix projection that determines the 3D surface point $\mathbf{P}(x, y)$ of each target lightmap sample and bilinearly interpolates the lighting value from the source lightmap using the original projection basis.
- **Pristine Lightmap Buffer**: An immutable or undo-tracked cache of original baked lightmap samples used as the ground truth source for resamplings to prevent generational loss.

---

## Requirements

### Requirement 1: Viewport Lightmap Stability During Real-Time Texture Editing
**User Story:** As a mapper adjusting texture scale, offset, or rotation in real-time, I want the face lighting in the 3D viewport to remain stationary on the surface rather than stretching, jumping, or displaying black/atlas borders.

#### Acceptance Criteria
1. WHEN the user drags texture sliders (`scaleX`, `scaleY`, `shiftX`, `shiftY`, `rotateX`, `rotateY`) in the Face Edit panel, THE viewport SHALL update surface texture UV coordinates (`vert.u`, `vert.v`) in real time.
2. DURING real-time texture slider dragging, THE lightmap coordinates (`vert.luv`) SHALL be calculated using the lightmap's existing projection basis (`LightmapInfo`) rather than the active, uncommitted `texinfo`.
3. THE system SHALL NOT corrupt lightmap UV coordinates when `fU` and `fV` deviate from initial values during interactive dragging.
4. WHERE `manualMode` is active or inactive, viewport lightmap rendering SHALL remain visually locked to the polygon's world-space position.

---

### Requirement 2: World-Space Invariant Lightmap Resampling in BSP Lighting Lump
**User Story:** As a mapper changing texture scale or shift on a face, I want the saved BSP lightmaps to retain the exact world-space lighting distribution baked by the RAD compiler, so that in-game lighting remains authentic and unshifted.

#### Acceptance Criteria
1. WHEN face texture scaling, shifting, or rotation changes the computed lightmap dimensions ($W_{\text{new}} \ne W_{\text{old}}$ or $H_{\text{new}} \ne H_{\text{old}}$) or sub-grid phase ($\text{shift} \bmod 16$), THE system SHALL calculate target lightmap samples using world-space 3D face plane projection.
2. FOR EACH target sample $(x_{\text{new}}, y_{\text{new}})$ on the target grid, THE system SHALL:
   1. Compute the target 2D texture coordinates $s_{\text{new}} = (\text{mins}_{\text{new}}[0] + x_{\text{new}}) \times 16$ and $t_{\text{new}} = (\text{mins}_{\text{new}}[1] + y_{\text{new}}) \times 16$.
   2. Solve the 3D world position $\mathbf{P} \in \mathbb{R}^3$ on the face plane using the inverse of the target texture projection matrix.
   3. Project $\mathbf{P}$ into the source texture space $s_{\text{orig}} = \mathbf{P} \cdot \mathbf{vS}_{\text{orig}} + \text{shiftS}_{\text{orig}}$ and $t_{\text{orig}} = \mathbf{P} \cdot \mathbf{vT}_{\text{orig}} + \text{shiftT}_{\text{orig}}$.
   4. Convert $(s_{\text{orig}}, t_{\text{orig}})$ to fractional coordinates on the source lightmap grid.
   5. Bilinearly sample the source lightmap color with edge clamping.
3. THE system SHALL NOT use naive $[0, 1] \to [0, 1]$ bounding-box image scaling (`scaleImage`) for face lightmap resizes.
4. IF texture vectors are rotated, THE system SHALL correctly sample the source lightmap across the rotated orientation without distortion or shearing.

---

### Requirement 3: Elimination of Generational Resampling Loss
**User Story:** As an editor user performing multiple consecutive adjustments on face textures, I want lighting to maintain sharpness and fidelity without degrading into blur with each adjustment.

#### Acceptance Criteria
1. THE system SHALL maintain reference source lightmap data for each face that is independent of intermediate resampled results during an editing session.
2. WHEN committing consecutive texture modifications on the same face, THE system SHALL resample from the session's baseline/undo lightmap buffer rather than recursively resampling degraded outputs.
3. THE system SHALL update the baseline lightmap buffer only when explicit undo states are created or new lightmap operations (such as lightmap painting or pasting) are performed.

---

### Requirement 4: Preservation on Copy/Paste and Downscale Operations
**User Story:** As a mapper copying texture styles across faces or downscaling embedded textures, I want lightmaps on affected faces to remain intact.

#### Acceptance Criteria
1. WHEN applying `pasteTextureStyle` (`Gui.cpp`), THE system SHALL resample target face lightmaps using world-space reprojection instead of generic image stretching.
2. WHEN downscaling embedded textures via `adjust_downscaled_texture_coordinates` (`Bsp.cpp`), THE system SHALL preserve face lighting alignment using world-space resampling.
3. WHEN applying texture replacements or textures with different dimensions from WADs in `GuiWidgets.cpp`, THE system SHALL keep lightmap lighting distribution aligned.

---

### Requirement 5: Model & Brush Transformation Consistency
**User Story:** As a mapper scaling brush entities or moving vertices, I want entity lightmap updates to preserve lighting continuity without artifacts.

#### Acceptance Criteria
1. WHEN scaling brush models in `Renderer::scaleSelectedObject`, THE system SHALL apply world-space aware lightmap resizing across all affected model faces.
2. WHEN splitting faces or carving geometry, newly generated sub-faces SHALL sample their lightmaps from the parent face's world-space lighting plane.

---

### Requirement 6: Performance & Memory Safety
**User Story:** As a user working on large maps with thousands of faces, I want lightmap adjustments to execute efficiently without stuttering or memory leaks.

#### Acceptance Criteria
1. World-space lightmap resampling for selected faces SHALL complete in under 15 ms per face.
2. `Bsp::resize_all_lightmaps` SHALL perform selective resampling only on faces whose lightmap grid dimensions or basis vectors have changed, avoiding unnecessary work on untouched faces.
3. All intermediate buffers and matrices SHALL be safely deallocated without leaking memory.
