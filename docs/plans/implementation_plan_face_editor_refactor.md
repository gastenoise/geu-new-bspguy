# Implementation Plan: Face Editor Interaction & Workflow Refactoring

This plan breaks down the refactoring of the Face Editor widget, eliminating the global "Edit Mode" toggle and implementing property-specific real-time and explicit apply semantics.

---

## Task Breakdown

- [x] 1. Core State & Header Cleanup (`Gui.h`, `GuiWidgets.cpp`)
  - [x] 1.1 Remove `manualMode` and `applyFaceChanges` flags from `src/editor/Gui.h`.
    - _Requirements: REQ-1.1, REQ-1.2_
  - [x] 1.2 Remove the `Edit Mode: [Manual / Real Time]` button and `UNAPPLIED CHANGES (Manual Mode)` warning banner in `drawFaceEditorWidget()` (`src/editor/gui/GuiWidgets.cpp`).
    - _Requirements: REQ-1.1, REQ-1.2, REQ-1.3_

- [x] 2. Real-Time Continuous Texture Sliders (`GuiWidgets.cpp`)
  - [x] 2.1 Refactor scale ($X, Y$), shift ($X, Y$), and rotation ($X, Y$) slider handlers to update `BSPTEXTUREINFO` and call `mapRenderer->updateFaceUVs()` every frame during active dragging.
    - _Requirements: REQ-2.1, REQ-2.2, REQ-2.4_
  - [x] 2.2 Trigger lightmap resize (`map->resize_all_lightmaps(true)`), lightmap sync (`mapRenderer->reloadLightmapsSync()`), and undo push (`"Edit face"`, `EDIT_MODEL_LUMPS`) strictly upon mouse release (`ImGui::IsItemDeactivatedAfterEdit()`).
    - _Requirements: REQ-2.3_

- [x] 3. Explicit Manual Texture Assignment (`Gui.cpp`, `GuiWidgets.cpp`)
  - [x] 3.1 Refactor the `APPLY` button and `Enter` keypress handler next to `textureName2` in `drawFaceEditorWidget()` to immediately resolve/load textures, assign to selected faces, update models, sync lightmaps, and push undo (`"Change Face Texture"`).
    - _Requirements: REQ-3.1_
  - [x] 3.2 Refactor `Gui::pasteTexture()` in `src/editor/Gui.cpp` to execute directly on `app->pickInfo.selectedFaces` with undo creation, making it fully independent of `drawFaceEditorWidget()` visibility.
    - _Requirements: REQ-3.3_
  - [x] 3.3 Ensure `Apply Selected Texture` in `drawTextureBrowser()` directly applies the selected texture to active selected faces.
    - _Requirements: REQ-3.2_

- [x] 4. Explicit Manual Vertex Coordinates Editor (`GuiWidgets.cpp`)
  - [x] 4.1 Isolate `edgeVerts` coordinate inputs ($X, Y, Z$) to modify only local GUI values, preventing immediate geometry mutation.
    - _Requirements: REQ-4.1, REQ-4.2_
  - [x] 4.2 Add `APPLY VERTS` (or `APPLY`) button next to `COPY VERTS` to write `edgeVerts` to `map->verts`, recompute face plane, refresh model geometry/UVs, resize lightmaps, and push full geometry undo state (`FL_PLANES | FL_VERTICES | ...`).
    - _Requirements: REQ-4.3, REQ-4.4_
  - [x] 4.3 Add `RESET VERTS` button next to `COPY VERTS` to reload original coordinates from `map->verts` into `edgeVerts`.
    - _Requirements: REQ-4.5, REQ-4.6_

- [x] 5. Surface Flags & Styles Verification (`GuiWidgets.cpp`)
  - [x] 5.1 Ensure `Special (TEX_SPECIAL)` checkbox immediately toggles flags and pushes undo.
    - _Requirements: REQ-5.1_
  - [x] 5.2 Ensure lightmap style adjustments (`tmpStyles`) commit cleanly on edit deactivation.
    - _Requirements: REQ-5.2_
  - [x] 5.3 Verify multi-face `Merge verts!` operates with its dedicated button.
    - _Requirements: REQ-5.3_

- [x] 6. Testing, Verification & Polish
  - [x] 6.1 Verify continuous slider dragging in 3D viewport for scale, shift, and rotation.
  - [x] 6.2 Verify texture assignment from input box, Texture Browser, and `Ctrl+V`.
  - [x] 6.3 Verify manual vertex editing, application with `APPLY VERTS`, discarding with `RESET VERTS`, and clipboard copy with `COPY VERTS`.
  - [x] 6.4 Verify Undo / Redo functionality for all operations.
