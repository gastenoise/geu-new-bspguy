# Design Document: Face Editor Architecture & Interaction Refactor

## Overview

The Face Editor panel (`Gui::drawFaceEditorWidget`) is the primary interface in `revamped-newbspguy` for manipulating BSP polygon faces, texture mapping, vertex positioning, dynamic lightmap styles, and surface flags. 

This document details the architectural redesign that removes the ambiguous global `Edit Mode: Real Time / Manual` toggle and establishes a clean, decoupled component architecture with explicit interaction models suited for each property type.

---

## System Architecture

### Component Map

| Component ID | Name | Subsystem / File | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-1** | Face Editor Widget | `src/editor/gui/GuiWidgets.cpp` | Renders face properties, handles continuous sliders, explicit apply buttons, and vertex editor inputs. | `Bsp`, `BspRenderer`, `Gui` |
| **COMP-2** | Texture Browser Dispatcher | `src/editor/gui/GuiWidgets.cpp`, `Gui.cpp` | Allows selecting map/WAD textures and applying them directly to the active face selection. | `Bsp`, `BspRenderer`, `Texture` |
| **COMP-3** | Real-Time UV Updater | `src/editor/BspRenderer.cpp` | Updates polygon vertex texture coordinates dynamically during slider interactions without affecting decoupled lightmaps. | `BspRenderer`, `GuiWidgets` |
| **COMP-4** | Vertex Geometry Committer | `src/editor/gui/GuiWidgets.cpp`, `src/bsp/Bsp.cpp` | Applies edited `edgeVerts` coordinates to `map->verts`, recomputes plane equations, updates bounding boxes, and commits full geometry undo states. | `Bsp`, `BspRenderer` |
| **COMP-5** | Texture Mutator & Loader | `src/editor/Gui.cpp`, `src/bsp/Bsp.cpp` | Resolves texture names against map miptex entries and loaded WAD archives, adds missing textures, and assigns `iTextureInfo`. | `Bsp`, `Wad`, `BspRenderer` |

---

### High-Level Component & Interaction Diagram

```
+---------------------------------------------------------------------------------------------------+
|                                      Face Editor Panel (UI)                                       |
+---------------------------------------------------------------------------------------------------+
|                                                                                                   |
|  [Scale / Shift / Rotate Sliders]              [Texture Name Input + APPLY]      [Vertex Editor]   |
|         (Continuous Interaction)                     (Explicit Action)          (Explicit Action) |
+------------------+------------------------------------------+----------------------------+---------+
                   |                                          |                            |
       [Real-Time Dragging]                                   |                            |
                   |                                  [Click APPLY / Enter]         [Click APPLY VERTS]
                   v                                          |                            |
+----------------------------------+                          v                            v
|    BspRenderer::updateFaceUVs    |             +-------------------------+  +---------------------+
|        (Live Viewport UVs)       |             |  Texture Mutator/Loader |  |  Vertex Committer   |
+------------------+---------------+             |        (COMP-5)         |  |      (COMP-4)       |
                   |                             +------------+------------+  +----------+----------+
          [Mouse Release]                                     |                          |
                   |                                          |                          |
                   v                                          v                          v
+---------------------------------------------------------------------------------------------------+
|                                     Commit & Synchronize                                          |
|  - Bsp::resize_all_lightmaps(true)                                                                |
|  - BspRenderer::reloadLightmapsSync()                                                             |
|  - BspRenderer::pushUndoState(...)                                                                |
|  - Refresh Model / Face Highlighting                                                              |
+---------------------------------------------------------------------------------------------------+
```

---

## Data Flow & Interaction Specifications

### 1. Continuous Alignment Sliders Flow (Scale, Shift, Rotate)

```
User drags DragFloat (scale/shift/rotate)
  │
  ├── Frame Update (while dragging):
  │     ├── texinfo = map->get_unique_texinfo(faceIdx)
  │     ├── Update texinfo->vS, texinfo->vT, texinfo->shiftS, texinfo->shiftT
  │     └── mapRenderer->updateFaceUVs(faceIdx)  [60+ FPS immediate viewport update]
  │
  └── Mouse Release (ImGui::IsItemDeactivatedAfterEdit):
        ├── map->resize_all_lightmaps(true)
        ├── mapRenderer->reloadLightmapsSync()
        ├── mapRenderer->pushUndoState("Edit face", EDIT_MODEL_LUMPS)
        └── Reset slider dirty flags
```

### 2. Explicit Texture Assignment Flow

```
User enters texture name and clicks "APPLY" (or selects from Texture Browser and clicks "Apply Selected Texture")
  │
  ├── 1. Lookup texture in map->textures:
  │        ├── Found: newMiptex = existing index
  │        └── Not found:
  │              ├── Search loaded WADs (mapRenderer->wads)
  │              │     └── If found: map->add_texture(name, wadData, w, h)
  │              └── Fallback: map->add_texture(name, randomColorImg, 256, 256)
  │
  ├── 2. For each selected face:
  │        ├── texinfo = map->get_unique_texinfo(faceIdx)
  │        └── texinfo->iMiptex = newMiptex
  │
  ├── 3. Synchronize Renderer & Lightmaps:
  │        ├── mapRenderer->reuploadTextures() (if new texture added)
  │        ├── mapRenderer->refreshModel(modelIdx)
  │        ├── mapRenderer->updateFaceUVs(faceIdx)
  │        ├── map->resize_all_lightmaps(true)
  │        └── mapRenderer->reloadLightmapsSync()
  │
  └── 4. Push Undo State:
           └── mapRenderer->pushUndoState("Change Face Texture", EDIT_MODEL_LUMPS)
```

### 3. Explicit Vertex Coordinates Editor Flow

```
User edits DragFloat/Input fields for V[i].x, V[i].y, V[i].z
  │
  ├── Local State:
  │     ├── edgeVerts[i] updated locally in memory
  │     └── vertsModified flag marked true
  │     (NO BSP modifications, NO viewport breakage during typing)
  │
  ├── IF User clicks "RESET":
  │     ├── Reload edgeVerts from map->verts
  │     └── vertsModified = false
  │
  └── IF User clicks "APPLY VERTS":
        ├── 1. For each edge in face:
        │        └── map->verts[vertexIdx] = edgeVerts[i]
        ├── 2. Recalculate face plane:
        │        └── map->planes[face.iPlane] = PlaneFromPoints(edgeVerts)
        ├── 3. Refresh models & render geometry:
        │        ├── mapRenderer->refreshModel(modelIdx)
        │        ├── mapRenderer->updateFaceUVs(faceIdx)
        │        ├── map->resize_all_lightmaps(true)
        │        └── mapRenderer->reloadLightmapsSync()
        ├── 4. Push Undo State:
        │        └── mapRenderer->pushUndoState("Edit Face Vertices", 
        │              FL_PLANES | FL_TEXTURES | FL_VERTICES | FL_NODES | FL_TEXINFO | 
        │              FL_FACES | FL_LIGHTING | FL_CLIPNODES | FL_LEAVES | FL_EDGES | 
        │              FL_SURFEDGES | FL_MODELS)
        └── 5. Reset vertsModified = false
```

---

## UI Layout Specifications

```
+--------------------------------------------------------------+
| Face Editor (Face #128)                                  [X] |
+--------------------------------------------------------------+
| Lightmap: 16x16 (256 samples)                                |
|                                                              |
| [ Texture Alignment ]                                        |
| Scale   X: [ 1.000 ]           Y: [ 1.000 ]                  |
| Shift   X: [ 0.000 ]           Y: [ 0.000 ]                  |
| Rotate  X: [ 0.000 ]           Y: [ 0.000 ]     [X] Lock     |
|                                                              |
| [ Texture Assignment ]                                       |
| Name: [ +0BUTTON     ] #4      [ APPLY ]      (128x128)      |
| +----------------------------------------------------------+ |
| |                    [ Texture Preview ]                   | |
| |              (Click to open Texture Browser)             | |
| +----------------------------------------------------------+ |
|                                                              |
| [ Surface Flags & Lighting ]                                 |
| [ ] Special (TEX_SPECIAL)                                    |
| Styles:  #1 [ 255 ]  #2 [ 255 ]  #3 [ 255 ]  #4 [ 255 ]      |
|                                                              |
| [ Vertex Coordinates ] (Single Face Only)                    |
| V1:  X [ 128.000 ]  Y [ 64.000  ]  Z [ 0.000   ]             |
| V2:  X [ 256.000 ]  Y [ 64.000  ]  Z [ 0.000   ]             |
| V3:  X [ 256.000 ]  Y [ 192.000 ]  Z [ 0.000   ]             |
| V4:  X [ 128.000 ]  Y [ 192.000 ]  Z [ 0.000   ]             |
|                                                              |
| [ COPY VERTS ]     [ APPLY VERTS ]     [ RESET VERTS ]       |
|                                                              |
| Lightmap Offset: 0x004A20                                    |
+--------------------------------------------------------------+
```

---

## Error Handling & Edge Cases

1. **Division by Zero in Scale**:
   - `std::isnan(scaleX) || std::abs(scaleX) < 0.00001f` checks prevent infinite or undefined texture vector coordinates.
2. **Invalid Texture Name**:
   - If an entered texture is neither in the map nor in loaded WADs, a generated placeholder is created safely without crashing.
3. **Collinear or Degenerate Vertices**:
   - Vertex coordinate edits validate that vertices define a non-degenerate planar polygon before plane recomputation.
4. **Window Closed during Paste Action**:
   - `pasteTexture()` executes synchronously on `app->pickInfo.selectedFaces`, functioning properly regardless of `showFaceEditWidget` visibility.
