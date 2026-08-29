# Requirements Document: Face Editor Refactoring & Granular Edit Semantics

## Introduction

In `revamped-newbspguy`, the Face Editor (`Gui::drawFaceEditorWidget`) currently features a global toggle button called `Edit Mode: Real Time / Manual`. This toggle creates UX ambiguity, redundant confirmation buttons, and critical bugs:
1. When in "Real Time" mode, manual vertex editing fails to assign modified vertices to the BSP while pushing empty undo states.
2. In "Manual" mode, clicking "APPLY" next to a texture name does not apply the texture until a separate top-level "APPLY CHANGES" button is clicked.
3. Interactive sliders (Scale, Shift, Rotate) and discrete actions (texture assignment, vertex coordinate updates) have fundamentally different workflows and should not be governed by a single global toggle.

This document defines the formal requirements to eliminate the global "Edit Mode" toggle and establish granular, property-specific editing semantics: real-time updates for continuous texture alignment sliders, explicit manual application for texture assignment and vertex manipulation, and direct synchronization with the 3D viewport, lightmap manager, and undo system.

---

## Glossary

- **`BSPTEXTUREINFO` (`texinfo`)**: BSP structure containing 3D texture projection vectors (`vS`, `vT`) and offset coordinates (`shiftS`, `shiftT`).
- **`BSPFACE32`**: BSP structure representing a polygon face, referencing vertices via surfedges and lighting properties.
- **Continuous Alignment Sliders**: Interactive UI controls for texture Scale ($X, Y$), Shift ($X, Y$), and Angles/Rotation ($X, Y$) that provide fluid viewport feedback on drag.
- **Explicit Application Actions**: Discrete operations (such as texture name assignment, texture browser application, and vertex coordinate editing) that require an explicit "Apply" action to prevent invalid intermediate states.
- **`edgeVerts`**: A local list of 3D coordinates representing the ordered polygon vertices of the selected face.
- **Undo State (`pushUndoState`)**: Snapshot of specific BSP lumps stored in the undo stack for reversible map operations.

---

## Requirements

### Requirement 1: Removal of Global "Edit Mode" Header
**User Story:** As a level designer, I want a clean and unambiguous Face Editor window without confusing "Manual vs Real Time" mode switches, so that I can edit faces intuitively without duplicate apply buttons.

#### Acceptance Criteria
1. THE Face Editor panel SHALL NOT display an `Edit Mode: [Manual / Real Time]` toggle button.
2. THE Face Editor panel SHALL NOT display an unapplied changes warning banner (`UNAPPLIED CHANGES (Manual Mode)`) or top-level `APPLY CHANGES` button.
3. ALL controls in the Face Editor SHALL adopt inherent, predictable interaction models (continuous live updates or explicit action triggers).

---

### Requirement 2: Real-Time Interactive Texture Alignment (Scale, Shift, Rotate)
**User Story:** As a mapper aligning textures, I want texture scaling, shifting, and rotation to update the 3D viewport in real time while dragging sliders, and commit an undo state upon releasing the mouse.

#### Acceptance Criteria
1. WHEN the user drags any slider for Scale ($X, Y$), Shift ($X, Y$), or Rotation ($X, Y$), THE system SHALL update the active face texture projection vectors (`vS`, `vT`, `shiftS`, `shiftT`) and call `updateFaceUVs` on every frame.
2. DURING interactive slider dragging, THE viewport SHALL render updated surface texture coordinates at interactive frame rates without stretching or corrupting world-space lightmaps.
3. WHEN the user releases the mouse button after dragging a slider (`IsItemDeactivatedAfterEdit`), THE system SHALL:
   1. Resize and synchronize face lightmaps (`resize_all_lightmaps(true)`, `reloadLightmapsSync`).
   2. Push an undo state with lump mask `EDIT_MODEL_LUMPS` and label `"Edit face"`.
   3. Reset slider dirty flags.
4. IF multiple faces are selected, THE system SHALL apply slider modifications across all selected faces simultaneously.

---

### Requirement 3: Explicit Manual Texture Assignment
**User Story:** As a mapper changing a face texture, I want texture changes from the text input field or Texture Browser to apply explicitly when I click "Apply", without requiring secondary confirmations.

#### Acceptance Criteria
1. WHEN the user enters a texture name in the Face Editor text input and clicks `APPLY` (or presses Enter), THE system SHALL:
   1. Locate or load the texture from internal map textures or external WAD archives (or generate a placeholder if missing).
   2. Assign the texture index (`iMiptex`) to all selected faces.
   3. Refresh model rendering and highlight selected faces.
   4. Synchronize lightmaps and push an undo state with label `"Change Face Texture"`.
   5. Update the preview image and texture dimensions displayed in the Face Editor.
2. WHEN the user selects a texture in the Texture Browser and clicks `Apply Selected Texture` (or double-clicks a thumbnail), THE system SHALL directly apply the selected texture to all active selected faces with an undo state, regardless of whether the Face Editor panel is currently visible.
3. WHEN the user triggers `Ctrl+V` or context menu `Paste Texture`, THE system SHALL apply the copied texture to all active selected faces with an undo state.

---

### Requirement 4: Explicit Manual Vertex Coordinates Editor
**User Story:** As an advanced mapper editing polygon vertices manually, I want to edit vertex coordinates in a local buffer and apply them only when I click "APPLY VERTS", preventing intermediate non-coplanar geometry corruption during typing or dragging.

#### Acceptance Criteria
1. WHEN a single face is selected, THE Face Editor SHALL display editable coordinate fields ($X, Y, Z$) for each vertex in `edgeVerts`.
2. EDITING coordinate fields SHALL modify only the local `edgeVerts` buffer without immediately mutating BSP geometry or pushing undo states.
3. THE system SHALL provide an `APPLY VERTS` (or `APPLY`) button adjacent to the `COPY VERTS` button.
4. WHEN the user clicks `APPLY VERTS`, THE system SHALL:
   1. Copy `edgeVerts` coordinates into the BSP vertex lump (`map->verts`).
   2. Recompute the face plane and validate polygon geometry.
   3. Refresh model geometry and update face UVs and lightmaps.
   4. Push an undo state with full geometry lump mask (`FL_PLANES | FL_TEXTURES | FL_VERTICES | FL_NODES | FL_TEXINFO | FL_FACES | FL_LIGHTING | FL_CLIPNODES | FL_LEAVES | FL_EDGES | FL_SURFEDGES | FL_MODELS`).
5. THE system SHALL provide a `RESET` button adjacent to `COPY VERTS` that reloads the original vertex coordinates from `map->verts`, discarding unapplied manual edits.
6. THE `COPY VERTS` button SHALL continue to format all `edgeVerts` to keyvalue strings and copy them to the system clipboard.

---

### Requirement 5: Immediate Toggle for Face Flags and Lightmap Styles
**User Story:** As a mapper adjusting special rendering flags or lightmap styles, I want checkbox and style edits to apply cleanly and commit undo states immediately.

#### Acceptance Criteria
1. WHEN the `Special (TEX_SPECIAL)` checkbox is toggled, THE system SHALL update `texinfo->nFlags`, refresh models, synchronize lightmaps, and push an undo state.
2. WHEN dynamic lightmap style fields (`#1` through `#4`) are modified, THE system SHALL update `face.nStyles` and push an undo state on edit deactivation.
3. THE `Merge verts!` action SHALL remain an explicit operation triggered by its dedicated button.
