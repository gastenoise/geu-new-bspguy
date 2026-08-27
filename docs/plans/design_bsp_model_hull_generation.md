# System Design Document: Universal BSP Model Collision Hull Generation

## Overview

This document specifies the technical design and architectural overhaul for generating, regenerating, and managing collision hulls (Hulls 1, 2, and 3) for any BSP model in `revamped-newbspguy`. It eliminates the artificial UI and algorithmic convexity barriers, rectifies recursive BSP-to-clipnode conversion bugs, and introduces a robust multi-strategy collision generator that seamlessly supports convex, concave, multi-faceted, and complex submodels.

---

## System Architecture

### Component Map

| Component ID | Name | Module | Responsibility | Interfaces With |
|---|---|---|---|---|
| **COMP-1** | Context Menu & UI Controller | `GuiContextMenu.cpp`, `GuiMenuBar.cpp` | Provides unblocked user controls for generating, simplifying, and inspecting hulls | COMP-2, COMP-3 |
| **COMP-2** | Collision Generator Engine | `Bsp.cpp`, `Bsp.h` | Core algorithms for converting visual BSP trees, face ladders, and AABB bounds into clipnodes | COMP-4, COMP-5 |
| **COMP-3** | Cull Entity Collision Bridge | `GuiMenuBar.cpp`, `Bsp.cpp` | Generates dedicated brush entities (`func_wall`, `func_monsterclip`) from cull box boundaries | COMP-2 |
| **COMP-4** | Lump Management & BSP Structures | `Bsp.cpp`, `bsptypes.h` | Allocates and writes `BSPCLIPNODE32`, `BSPPLANE`, and updates `BSPMODEL` headnodes | Disk I/O, Undo System |
| **COMP-5** | Viewport Collision Renderer | `BspRenderer.cpp`, `Renderer.cpp` | Rebuilds OpenGL VBOs and wireframe buffers for real-time collision visualizer | OpenGL, Camera |

---

### High-Level Architecture Diagram

```
+-----------------------------------------------------------------------+
|                         User Interface Layer                          |
|  - Entity Right-Click Context Menu (Hulls -> Create / Simplify)       |
|  - Cull Actions Menu (Create Solid / Monsterclip Entity in Box)       |
+-----------------------------------+-----------------------------------+
                                    |
                                    v
+-----------------------------------------------------------------------+
|                    Collision Generator Engine (Bsp)                   |
|                                                                       |
|  Strategy 1: Universal Node-to-Clipnode Converter                     |
|  - Full BSP hierarchy recursion with plane expansion                  |
|  - Dual-branch traversal (preserves all internal nodes)               |
|                                                                       |
|  Strategy 2: Face-Ladder Direct Polygon Generator                     |
|  - Polygon plane extraction + Minkowski normal expansion              |
|  - Bounding box encapsulation                                         |
|                                                                       |
|  Strategy 3: Bounding Box AABB Hull Generator                         |
|  - Fast 6-clipnode collision with hull-specific extent offsets        |
+-----------------------------------+-----------------------------------+
                                    |
            +-----------------------+-----------------------+
            v                                               v
+-----------------------+                       +-----------------------+
|    BSP Data Layer     |                       |  OpenGL Render Layer  |
| - LUMP_CLIPNODES      |                       | - Clipnode Buffers    |
| - LUMP_PLANES         |                       | - Wireframe Buffers   |
| - BSPMODEL.iHeadnodes |                       | - Model Refresh       |
| - Undo Stack Tracking |                       | - Console Logging     |
+-----------------------+                       +-----------------------+
```

---

## Data Flow Specifications

### Primary Flow 1: Universal Collision Generation from Visual Model Tree

```
1. User triggers "Hulls -> Create Hull -> Clipnodes / Hull X"
   ↳ Context menu invokes `Bsp::generate_model_clipnodes(int modelIdx, int hullIdx, int strategy)`

2. Algorithm checks if model has a valid visual node tree (`model.iHeadnodes[0] >= 0`)
   ↳ Yes: Executes recursive `convert_nodes_to_clipnodes(iNode, hullIdx)`

3. For each BSP node encountered:
   ↳ Extract `nodePlane = planes[node.iPlane]`
   ↳ Calculate expansion offset: `extent = dotProduct(|normal|, default_hull_extents[hullIdx])`
   ↳ Allocate new `BSPPLANE` with expanded distance
   ↳ Allocate new `BSPCLIPNODE32` pointing to expanded plane
   ↳ Recursively convert `node.iChildren[0]` and `node.iChildren[1]`

4. For leaf endpoints (`iChildren[k] < 0`):
   ↳ Visual leaf `nContents == CONTENTS_SOLID` → clipnode child = `CONTENTS_SOLID (-2)`
   ↳ Visual leaf `nContents == CONTENTS_EMPTY` → clipnode child = `CONTENTS_EMPTY (-1)`
   ↳ Custom contents preserved (water, slime, lava)

5. Encapsulate root with outer bounding box clipnodes (prevents infinite boundary leakage)
   ↳ Store root clipnode index in `models[modelIdx].iHeadnodes[hullIdx]`

6. Perform lump pointer synchronization & cleanup (`remove_unused_model_structures`)
   ↳ Trigger `BspRenderer::refreshModelClipnodes(modelIdx)`
   ↳ Push undo state: `EDIT_MODEL_LUMPS | FL_CLIPNODES | FL_PLANES | FL_MODELS`
   ↳ Log completion details to console
```

---

### Primary Flow 2: Face-Ladder Collision Generation from Model Polygons

```
1. Fallback or explicit trigger for models without hierarchical node trees:
   ↳ Collect all faces `BSPFACE32` for `modelIdx` (`iFirstFace` .. `iFirstFace + nFaces - 1`)

2. Deduplicate unique face planes:
   ↳ Normalize plane directions and filter duplicates within angular/distance epsilon

3. Generate outer bounding box:
   ↳ `create_clipnode_box(model.nMins, model.nMaxs, &model, hullIdx, false)`

4. Append expanded face plane ladder:
   ↳ For each unique face plane:
     - Expand plane by `offset = dotProduct(|normal|, default_hull_extents[hullIdx])`
     - Create `BSPCLIPNODE32`
     - Outside side → `CONTENTS_EMPTY`
     - Inside side → Next clipnode index in sequence (last points to `CONTENTS_SOLID`)

5. Link last bounding box node to face ladder head
   ↳ Update model headnode and refresh viewport
```

---

### Primary Flow 3: Cull Area Solid Brush Entity Creation

```
1. User defines cull box with 2 `cull` entities
   ↳ Menu triggers "Create Solid Brush Entity in Cull Box"

2. Retrieve world coordinates `mins = min(cullMins, cullMaxs)`, `maxs = max(cullMins, cullMaxs)`
   ↳ Model dimensions: `size = maxs - mins`, `origin = (mins + maxs) * 0.5`
   ↳ Local coordinates: `localMins = -size * 0.5`, `localMaxs = size * 0.5`

3. Create submodel with visual faces and collision clipnodes:
   ↳ `newModelIdx = map->create_solid(localMins, localMaxs, triggerOrNullTex, false)`

4. Create entity (`func_wall` or `func_monsterclip`):
   ↳ Add keyvalue `"origin", origin.toKeyvalueString()`
   ↳ Add keyvalue `"model", "*" + std::to_string(newModelIdx)`
   ↳ Add keyvalue `"classname", "func_wall"`

5. Insert into `map->ents`, refresh lightmaps, and update entity list
```

---

## Detailed Component Specifications

### 1. UI Unblocking & Menu Overhaul (`src/editor/gui/GuiContextMenu.cpp`)

#### Changes in `GuiContextMenu.cpp`:
- **Remove blocking condition**: Change `!app->invalidSolid && app->isTransformableSolid` to `!app->isLoading && (modelIdx > 0 || map->is_bsp_model)`.
- **Fix "Simplify Hull" enablement**: Change `isHullValid` condition so the root "Simplify Hull" menu and its sub-items are active whenever `!app->isLoading`, allowing creation of initial bounding-box collision.
- **Fix hardcoded hull index bug**: Change `map->simplify_model_collision(modelIdx, 1)` in the loop to `map->simplify_model_collision(modelIdx, i)`.

```cpp
// Corrected Context Menu Structure in GuiContextMenu.cpp:
if (ImGui::BeginMenu(get_localized_string(LANG_0457).c_str(), !app->isLoading && (modelIdx > 0 || map->is_bsp_model))) // "Create Hull"
{
    if (ImGui::MenuItem(get_localized_string(LANG_0458).c_str())) // "All Clipnodes (1-3)"
    {
        map->regenerate_model_clipnodes_universal(modelIdx, -1);
        rend->refreshModelClipnodes(modelIdx);
        checkValidHulls();
    }
    ImGui::Separator();
    for (int i = 1; i < MAX_MAP_HULLS; i++)
    {
        if (ImGui::MenuItem(("Hull " + std::to_string(i)).c_str()))
        {
            map->regenerate_model_clipnodes_universal(modelIdx, i);
            rend->refreshModelClipnodes(modelIdx);
            checkValidHulls();
        }
    }
    ImGui::EndMenu();
}
```

---

### 2. Universal Model Clipnode Generator (`src/bsp/Bsp.cpp`, `src/bsp/Bsp.h`)

#### New / Overhauled Methods in `Bsp`:

```cpp
// In Bsp.h:
bool regenerate_model_clipnodes_universal(int modelIdx, int hullIdx);
int convert_nodes_to_clipnodes_recursive(int iNode, int hullIdx, bool& success);
bool generate_clipnodes_from_model_faces(int modelIdx, int hullIdx);
```

#### Recursive Converter Algorithm (`Bsp.cpp`):

```cpp
int Bsp::convert_nodes_to_clipnodes_recursive(int iNode, int hullIdx, bool& success)
{
    if (iNode < 0 || iNode >= nodeCount)
    {
        success = false;
        return CONTENTS_EMPTY;
    }

    BSPNODE32& node = nodes[iNode];
    if (node.iPlane < 0 || node.iPlane >= planeCount)
    {
        success = false;
        return CONTENTS_EMPTY;
    }

    // Allocate new clipnode and duplicate expanded plane
    int newClipIdx = create_clipnode();
    BSPPLANE origPlane = planes[node.iPlane];
    
    vec3 hullExtent = default_hull_extents[hullIdx];
    vec3 absNormal = vec3(std::abs(origPlane.vNormal.x), std::abs(origPlane.vNormal.y), std::abs(origPlane.vNormal.z));
    float offset = dotProduct(absNormal, hullExtent);

    BSPPLANE expandedPlane = origPlane;
    expandedPlane.fDist += offset;

    int newPlaneIdx = create_plane();
    planes[newPlaneIdx] = expandedPlane;
    clipnodes[newClipIdx].iPlane = newPlaneIdx;

    // Process both children recursively (preserving full branching BSP tree)
    for (int k = 0; k < 2; k++)
    {
        if (node.iChildren[k] >= 0)
        {
            clipnodes[newClipIdx].iChildren[k] = convert_nodes_to_clipnodes_recursive(node.iChildren[k], hullIdx, success);
        }
        else
        {
            int leafIdx = ~node.iChildren[k];
            if (leafIdx >= 0 && leafIdx < leafCount)
            {
                int contents = leaves[leafIdx].nContents;
                clipnodes[newClipIdx].iChildren[k] = (contents == CONTENTS_SOLID) ? CONTENTS_SOLID : CONTENTS_EMPTY;
            }
            else
            {
                clipnodes[newClipIdx].iChildren[k] = CONTENTS_EMPTY;
            }
        }
    }

    return newClipIdx;
}
```

---

## Error Handling & Edge Cases

| Edge Case | Risk | Mitigation Strategy |
|---|---|---|
| **Zero visual faces (`nFaces == 0`)** | Model has no visual geometry | Fall back to bounding-box hull generation using `model.nMins`/`model.nMaxs`. |
| **Invalid headnode (`iHeadnodes[0] < 0`)** | Missing visual BSP tree | Fall back to polygon face-ladder generator (`generate_clipnodes_from_model_faces`). |
| **Complex non-convex / hollow shapes** | Player gets stuck in concave seams | Encapsulate outer bounds with bounding box clipnodes to prevent rays escaping. |
| **Clipnode lump capacity overflow** | Map reaches maximum clipnode count | Pre-check lump bounds before allocation; trigger garbage collection (`remove_unused_model_structures`). |

---

## Testing Strategy

1. **Test Case 1: Simple Cube Brush Model**
   - Create a `func_wall` with 6 faces.
   - Run "Create Hull -> All Clipnodes". Verify in viewport (Toggle Hull 1/2/3) that wireframe boxes match expanded dimensions.
2. **Test Case 2: Multi-Faceted Convex Brush (Cylinder / Prism / Wedge)**
   - Select a model with 12–24 faces.
   - Verify "Create Hull" is enabled in context menu.
   - Generate hulls and verify player cannot pass through any side.
3. **Test Case 3: Concave / Branching Submodel (Arch / L-Shape / Staircase)**
   - Select a non-convex brush entity.
   - Verify "Create Hull" converts all branches without deleting half the model.
4. **Test Case 4: Zero-Collision Model Recovery**
   - Take a model with `iHeadnodes[1..3] == -1`.
   - Run "Simplify Hull -> Hull 1", "Hull 2", "Hull 3".
   - Verify clean 6-clipnode boxes are generated for each specific hull.
5. **Test Case 5: Cull Box Entity Generation**
   - Place 2 cull entities.
   - Trigger "Create Solid Entity in Box". Verify `func_wall` appears at box extents with complete collision.
