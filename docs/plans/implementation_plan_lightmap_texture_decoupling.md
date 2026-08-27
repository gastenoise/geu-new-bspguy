# Implementation Plan: Decoupled Lightmap Preservation

This implementation plan outlines the hierarchical task breakdown for developing, integrating, and verifying the decoupled, world-space-accurate lightmap preservation pipeline across `revamped-newbspguy`.

---

## Task Hierarchy & Checklists

- [x] 1. Viewport Lightmap Decoupling & Real-Time Stability
  - [x] 1.1 Extend `LightmapInfo` struct in `src/editor/BspRenderer.h` with original projection vectors (`vS`, `vT`, `shiftS`, `shiftT`, `imins`, `imaxs`).
    - _Requirements: REQ-1.1, REQ-1.2_
    - _Dependencies: None_
  - [x] 1.2 Update `BspRenderer::loadLightmaps()` in `src/editor/BspRenderer.cpp` to populate the new `LightmapInfo` projection basis fields when packing atlases.
    - _Requirements: REQ-1.2_
    - _Dependencies: Task 1.1_
  - [x] 1.3 Modify `BspRenderer::updateFaceUVs()` in `src/editor/BspRenderer.cpp` to compute `vert.luv` using `lmap->vS` and `lmap->shiftS` instead of active `texinfo`.
    - _Requirements: REQ-1.1, REQ-1.2, REQ-1.3, REQ-1.4_
    - _Dependencies: Task 1.2_
  - [x] 1.4 Test real-time dragging in the Face Edit panel (`GuiWidgets.cpp`) to verify viewport lightmaps remain stationary during scale/shift/rotate slider interactions.
    - _Requirements: REQ-1.4_
    - _Dependencies: Task 1.3_

- [x] 2. World-Space Invariant Lightmap Resampling Engine
  - [x] 2.1 Enhance `LIGHTMAP` struct in `src/bsp/Bsp.h` to store projection basis vectors (`vS`, `vT`, `shiftS`, `shiftT`) and extents (`imins`, `imaxs`).
    - _Requirements: REQ-2.1, REQ-3.1_
    - _Dependencies: Phase 1_
  - [x] 2.2 Update `Bsp::save_undo_lightmaps()` in `src/bsp/Bsp.cpp` to capture baseline lightmap projection vectors and raw data alongside dimensions.
    - _Requirements: REQ-3.1, REQ-3.3_
    - _Dependencies: Task 2.1_
  - [x] 2.3 Implement `Bsp::resample_face_lightmap_world_space()` in `src/bsp/Bsp.cpp` computing 3D plane projection, matrix inversion, and bilinear interpolation with edge clamping.
    - _Requirements: REQ-2.1, REQ-2.2, REQ-2.3, REQ-2.4, REQ-6.1_
    - _Dependencies: Task 2.2_
  - [x] 2.4 Refactor `Bsp::resize_all_lightmaps()` in `src/bsp/Bsp.cpp` to replace naive `scaleImage()` calls with `resample_face_lightmap_world_space()`.
    - _Requirements: REQ-2.1, REQ-2.3, REQ-3.2, REQ-6.2_
    - _Dependencies: Task 2.3_

- [x] 3. Editor & GUI Integration
  - [x] 3.1 Update `Gui::pasteTextureStyle()` in `src/editor/Gui.cpp` to trigger world-space lightmap resizing.
    - _Requirements: REQ-4.1_
    - _Dependencies: Phase 2_
  - [x] 3.2 Update `Bsp::adjust_downscaled_texture_coordinates()` in `src/bsp/Bsp.cpp` to preserve lightmap alignment when textures are resized or downscaled.
    - _Requirements: REQ-4.2_
    - _Dependencies: Phase 2_
  - [x] 3.3 Update `Renderer::scaleSelectedObject()` and model vertex transformation logic in `src/editor/Renderer.cpp` to utilize world-space lightmap updates.
    - _Requirements: REQ-5.1, REQ-5.2_
    - _Dependencies: Phase 2_
  - [x] 3.4 Ensure `GuiWidgets.cpp` Face Edit panel properly triggers commit and atlas reloading without redundant resamplings.
    - _Requirements: REQ-1.4, REQ-6.2_
    - _Dependencies: Task 3.1_

- [x] 4. Verification, Testing & QA
  - [x] 4.1 Verify single-face scale/shift/rotate in both Real Time and Manual modes in `GuiWidgets.cpp`.
    - _Requirements: REQ-1.1, REQ-1.4, REQ-2.1_
    - _Dependencies: Phase 3_
  - [x] 4.2 Verify multi-face batch editing across different planes and angles.
    - _Requirements: REQ-2.4, REQ-6.2_
    - _Dependencies: Phase 3_
  - [x] 4.3 Verify BSP save/load cycle and inspect in Half-Life / GoldSrc engine to confirm no surface extent crashes or light shifting.
    - _Requirements: REQ-2.2, REQ-4.1, REQ-6.1_
    - _Dependencies: Phase 3_
