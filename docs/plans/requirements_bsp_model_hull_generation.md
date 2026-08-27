# Requirements Document: Universal BSP Model Collision Hull Generation

## Introduction

In the GoldSrc engine (Half-Life, Counter-Strike, Sven Co-op) and Quake BSP architecture, brush entities (`func_wall`, `func_door`, `func_illusionary`, etc.) and standalone submodels reference sub-models in the BSP file (`BSPMODEL`, e.g., `*1`, `*2`). Each submodel contains headnodes for its visual render tree (Hull 0) and three collision hulls (Hull 1 for standing player, Hull 2 for large monsters, Hull 3 for ducking players).

In the current implementation of `revamped-newbspguy`, mappers who import, create, or edit multi-faceted or non-cubic brush models find the **"Create Hull"** and **"Simplify Hull"** menu options permanently disabled (greyed out), or encounter corrupted collision structures upon generation. This forces mappers into workarounds such as using Cull entities, which also fail to generate collision.

This document establishes the comprehensive requirements to unlock, overhaul, and provide universal collision hull generation for any BSP model regardless of facet count, concavity, or geometric complexity.

---

## Glossary

- **`BSPMODEL`**: BSP structure representing worldspawn (`models[0]`) or a submodel (`models[1..N]`), referencing bounds `nMins`/`nMaxs`, visual face ranges `iFirstFace`/`nFaces`, and headnode indices `iHeadnodes[4]`.
- **Hull 0**: The visual BSP tree consisting of `BSPNODE32` nodes and `BSPLEAF32` leaves with rendered polygons, lightmaps, and textures.
- **Hulls 1, 2, 3**: The collision hull trees consisting of `BSPCLIPNODE32` clipnodes.
  - **Hull 1**: Standard human player hull (extent 32×32×72 units).
  - **Hull 2**: Large monster hull (extent 64×64×72 units).
  - **Hull 3**: Ducking player hull (extent 32×32×36 units).
- **`BSPCLIPNODE32`**: A binary space partitioning node for collision containing a splitting plane index `iPlane` and two children `iChildren[2]` (pointing to positive child clipnode indices or negative contents values `CONTENTS_EMPTY = -1`, `CONTENTS_SOLID = -2`).
- **`invalidSolid`**: A UI state flag originally intended to indicate non-planar vertex manipulation validity, but erroneously coupled as a blocker for collision generation.
- **Hull Expansion (Minkowski Sum)**: Expanding a polygon's splitting plane along its surface normal by the hull extent $\Delta d = |\mathbf{n}_x| E_x + |\mathbf{n}_y| E_y + |\mathbf{n}_z| E_z$ so that a point-sized trace represents collision against an expanded bounding box.
- **Face Ladder**: A sequential clipnode subtree constructed by evaluating all bounding planes of a model in series.

---

## Requirements

### Requirement 1: Unconditional UI Access for Model Collision Operations
**User Story:** As a mapper selecting any BSP brush model in the editor, I want the collision generation and hull management context menu to be enabled regardless of whether the model has 6 faces, 50 faces, or is non-convex, so that I can generate or regenerate collision without arbitrary blocks.

#### Acceptance Criteria
1. WHEN a user right-clicks on any submodel entity (`modelIdx > 0` or standalone BSP model), THE context menu item **"Hulls" -> "Create Hull"** (`LANG_0457`) SHALL be enabled and interactive whenever `!app->isLoading`.
2. THE system SHALL NOT disable the "Create Hull" menu based on the `invalidSolid` or `isTransformableSolid` vertex-editing flags.
3. THE system SHALL provide distinct hull generation actions:
   - **All Clipnodes (Hulls 1–3)**: Generates collision for all collision hulls.
   - **Hull 1 (Human Player)**: Generates or updates Hull 1 only.
   - **Hull 2 (Large Monster)**: Generates or updates Hull 2 only.
   - **Hull 3 (Ducking Player)**: Generates or updates Hull 3 only.
4. WHEN opening the **"Simplify Hull"** menu (`LANG_0461`), THE menu SHALL be enabled even when `model.iHeadnodes[i] < 0`, allowing mappers to generate a clean bounding-box hull for models lacking prior collision.
5. WHEN selecting an individual hull under "Simplify Hull", THE system SHALL pass the specific selected hull index `i` rather than hardcoding Hull 1.

---

### Requirement 2: Universal BSP Visual Tree to Collision Tree Conversion (Node-to-Clipnode)
**User Story:** As a mapper with a complex or multi-faceted brush model, I want the system to generate an exact collision hull by converting the model's visual node hierarchy into clipnodes with proper plane expansions, so that in-game collision matches the complex visual geometry.

#### Acceptance Criteria
1. WHEN regenerating collision from a model's visual tree (`models[modelIdx].iHeadnodes[0]`), THE system SHALL recursively convert all `BSPNODE32` nodes into corresponding `BSPCLIPNODE32` structures.
2. FOR EACH node in the tree, THE system SHALL create an expanded collision plane whose distance is adjusted by the dot product of the plane normal and the hull extent:
   $$\text{dist}_{\text{expanded}} = \text{dist}_{\text{orig}} + \text{sign} \cdot (|\mathbf{n}_x| E_x + |\mathbf{n}_y| E_y + |\mathbf{n}_z| E_z)$$
3. THE recursive converter SHALL traverse and preserve BOTH children (`iChildren[0]` and `iChildren[1]`) across all plane types, without discarding subtrees on axial planes (`PLANE_X`, `PLANE_Y`, `PLANE_Z`).
4. WHEN reaching a leaf node (`iChildren[k] < 0`):
   - IF the visual leaf has `nContents == CONTENTS_SOLID` (or `sharedSolidLeaf`), THE clipnode child SHALL be set to `CONTENTS_SOLID (-2)`.
   - IF the visual leaf has `nContents == CONTENTS_EMPTY`, THE clipnode child SHALL be set to `CONTENTS_EMPTY (-1)`.
   - IF the visual leaf has custom contents (`CONTENTS_WATER`, `CONTENTS_SLIME`, `CONTENTS_LAVA`), THE clipnode child SHALL preserve the corresponding negative contents identifier.
5. THE root of the converted clipnode tree SHALL be enclosed within a bounding box clipnode tree to prevent infinite ray extensions beyond model bounds.

---

### Requirement 3: Face-Ladder Collision Hull Generator (Direct Polygon Fallback)
**User Story:** As a mapper with an imported or edited BSP model whose visual node tree is degraded or absent, I want the system to generate a valid collision hull directly from the model's face polygon planes, so that every face creates a physical collision barrier.

#### Acceptance Criteria
1. WHEN visual node regeneration is unavailable or requested via face mode, THE system SHALL iterate over all faces (`BSPFACE32`) assigned to `models[modelIdx]` (`iFirstFace` to `iFirstFace + nFaces - 1`).
2. FOR EACH face plane, THE system SHALL compute the expanded plane using the face's normal and plane side orientation.
3. THE system SHALL construct an expanded face ladder inside the model's bounding box clipnodes, terminating in `CONTENTS_SOLID` at the innermost leaf and `CONTENTS_EMPTY` on all outward half-spaces.
4. THE system SHALL deduplicate identical and near-parallel planes to minimize clipnode count.

---

### Requirement 4: Bounding Box Hull Generator (Simplified Collision)
**User Story:** As a mapper optimizing collision for intricate models (such as decorative meshes, fences, or complex machinery), I want to generate a 6-plane axial bounding-box collision hull with a single click, so that the engine has minimal collision overhead.

#### Acceptance Criteria
1. WHEN the user triggers "Simplify Hull" for a model, THE system SHALL calculate the vertex bounds $[ \mathbf{v}_{\min}, \mathbf{v}_{\max} ]$ of `models[modelIdx]`.
2. THE system SHALL construct a 6-clipnode axial bounding box with extents expanded by `default_hull_extents[hullIdx]`.
3. THE generated bounding box clipnode index SHALL be assigned directly to `models[modelIdx].iHeadnodes[hullIdx]`.
4. THE system SHALL clean up any unused clipnodes and planes left behind from the prior collision tree.

---

### Requirement 5: Cull Area Collision Entity Creation (Cull Tool Fix)
**User Story:** As a mapper defining a rectangular cull box with two `cull` entities, I want the option to instantly generate a collision brush entity (`func_wall` or `func_monsterclip`) filling that exact area, so that I can manually place solid barriers without BSP node failures.

#### Acceptance Criteria
1. WHEN 2 `cull` entities define a valid box `[cullMins, cullMaxs]`, THE menu SHALL offer "Create Solid Entity in Box (`func_wall`)" and "Create Monsterclip Entity in Box".
2. THE system SHALL create a new `BSPMODEL` using `create_solid(cullMins, cullMaxs, triggerIdx, false)` and attach a new entity with origin at the box center.
3. WHEN invoking worldmodel `delete_hull_in_box`, THE system SHALL log clear progress and statistics to the console indicating the number of leaves evaluated and modified.

---

### Requirement 6: Viewport Synchronization, Logging & Undo Safety
**User Story:** As an editor user, I want the 3D viewport and console to immediately reflect newly created collision hulls, and I want full undo/redo support for all hull operations.

#### Acceptance Criteria
1. UPON completing hull generation, THE system SHALL automatically call `BspRenderer::refreshModelClipnodes(modelIdx)` to rebuild OpenGL wireframe and solid clipnode buffers.
2. THE system SHALL log a structured message to the console in the format:
   `[Collision] Model {ID}: Generated Hull {HullID} ({N} clipnodes, {P} planes) in {T}ms.`
3. THE system SHALL push an undo state with flags `EDIT_MODEL_LUMPS | FL_CLIPNODES | FL_PLANES | FL_MODELS`.
4. WHEN saving the map (`Ctrl+S`), THE BSP file on disk SHALL store all new clipnodes, planes, and updated model headnode indices.
