# Design Document: Decoupled Lightmap Preservation Architecture

## Overview

In GoldSrc / Quake BSP format (BSP30), each face polygon uses a shared `BSPTEXTUREINFO` record (`vS`, `vT`, `shiftS`, `shiftT`) for both texture mapping and lightmap sample grid generation. When a face's texture scale, shift, or rotation is changed, the engine expects the lightmap lump (`lightdata`) to contain samples matching the newly computed grid dimensions $W_{\text{new}} \times H_{\text{new}}$ and origin $\mathbf{s}_0^{\text{new}}$.

This document specifies the technical design to decouple real-time viewport lightmap rendering from active texture adjustments and implement exact world-space geometric lightmap resampling for BSP lightmap resizing.

---

## System Architecture

### Component Map

| Component ID | Name | Subsystem / File | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-1** | `WorldLightmapResampler` | `src/bsp/Bsp.cpp`, `rad.cpp` | Implements exact 3D planar world-space bilinear lightmap resampling between arbitrary source and target projection bases. | `Bsp`, `rad`, `LIGHTMAP` |
| **COMP-2** | `LightmapInfo` Projection Basis | `src/editor/BspRenderer.h` | Stores fixed lightmap projection vectors (`vS`, `vT`, `shiftS`, `shiftT`, `imins`) per face at atlas-bake time to decouple real-time UV edits. | `BspRenderer`, `Renderer` |
| **COMP-3** | Real-Time UV Pipeline | `src/editor/BspRenderer.cpp` | Updates vertex texture UVs (`u`, `v`) dynamically without corrupting vertex lightmap coordinates (`luv`). | `BspRenderer`, `GuiWidgets` |
| **COMP-4** | Baseline Lightmap Manager | `src/bsp/Bsp.h`, `Bsp.cpp` | Retains pristine lightmap data before interactive sessions to prevent generational resampling degradation during consecutive slider drags. | `Bsp`, `GuiWidgets`, `Gui` |
| **COMP-5** | Face & Style UI Integration | `src/editor/gui/GuiWidgets.cpp`, `Gui.cpp` | Connects Face Edit sliders and copy/paste style actions to the decoupled pipeline. | `Bsp`, `BspRenderer`, `Gui` |

---

### High-Level Architecture & Mathematical Model

```
+---------------------------------------------------------------------------------------------------+
|                                      Face Edit Panel (UI)                                         |
|  - Sliders: scaleX, scaleY, shiftX, shiftY, rotateX, rotateY                                      |
+---------------------------------+-----------------------------------------------------------------+
                                  |
            [Real-Time Dragging]  |  [Mouse Release / Commit / Apply]
                                  v
+---------------------------------+---------------------------------+
|                                 |                                 |
v                                 v                                 v
+-----------------------------+   +-----------------------------------------------------------------+
|   BspRenderer::updateFaceUVs |   |                    Bsp::resize_all_lightmaps                    |
|   (COMP-2 & COMP-3)         |   |                    (COMP-1 & COMP-4)                            |
|                             |   |                                                                 |
| 1. Compute vertex (u, v)    |   | 1. Calculate new lightmap bounds (W_new, H_new, mins_new).      |
|    using NEW texinfo.       |   | 2. Fetch baseline/pristine lightmap data (size, mins_orig, ti). |
|                             |   | 3. FOR EACH sample (x_new, y_new) in target grid:               |
| 2. Compute vertex luv using |   |    a. s_new = (mins_new[0] + x_new) * 16                        |
|    FIXED LightmapInfo basis |   |       t_new = (mins_new[1] + y_new) * 16                        |
|    (Stationary in 3D space) |   |    b. P_world = InvertMatrix(ti_new, plane) * (s_new, t_new, d) |
|                             |   |    c. s_orig = DotProduct(P_world, ti_orig.vS) + ti_orig.shiftS |
| -> NO viewport glitching    |   |       t_orig = DotProduct(P_world, ti_orig.vT) + ti_orig.shiftT |
+-----------------------------+   |    d. u_orig = (s_orig - mins_orig[0] * 16) / 16.0              |
                                  |       v_orig = (t_orig - mins_orig[1] * 16) / 16.0              |
                                  |    e. Sample bilinear color from baseline lightmap buffer.       |
                                  | 4. Update face.nLightmapOffset and replace lump.                |
                                  | 5. Reload atlas via BspRenderer::loadLightmaps().               |
                                  +-----------------------------------------------------------------+
```

---

## Detailed Component Specifications

### 1. Geometric World-Space Lightmap Resampling (`COMP-1`)

Given a face with plane $\mathbf{n} \cdot \mathbf{x} = d$, original texture basis $\mathbf{T}_{\text{orig}} = (\mathbf{vS}_{\text{orig}}, \text{shiftS}_{\text{orig}}, \mathbf{vT}_{\text{orig}}, \text{shiftT}_{\text{orig}})$, and new texture basis $\mathbf{T}_{\text{new}} = (\mathbf{vS}_{\text{new}}, \text{shiftS}_{\text{new}}, \mathbf{vT}_{\text{new}}, \text{shiftT}_{\text{new}})$:

#### Step 1: Matrix Inversion
Construct the affine transformation matrix $\mathbf{M}_{\text{new}}$ mapping 3D world space to $(s_{\text{new}}, t_{\text{new}}, \text{depth})$:
$$\mathbf{M}_{\text{new}} = \begin{bmatrix} \mathbf{vS}_{\text{new}.x} & \mathbf{vS}_{\text{new}.y} & \mathbf{vS}_{\text{new}.z} & \text{shiftS}_{\text{new}} \\ \mathbf{vT}_{\text{new}.x} & \mathbf{vT}_{\text{new}.y} & \mathbf{vT}_{\text{new}.z} & \text{shiftT}_{\text{new}} \\ \mathbf{n}_x & \mathbf{n}_y & \mathbf{n}_z & -d \\ 0 & 0 & 0 & 1 \end{bmatrix}$$

Invert $\mathbf{M}_{\text{new}}$ to obtain $\mathbf{M}_{\text{new}}^{-1}$ which projects any $(s, t, 0)$ back to 3D world coordinates on the face plane:
$$\mathbf{P}_{\text{world}}(s, t) = \mathbf{M}_{\text{new}}^{-1} \begin{bmatrix} s \\ t \\ 0 \\ 1 \end{bmatrix}$$

#### Step 2: Sampling Each Target Pixel
For each pixel $(x, y) \in [0, W_{\text{new}}-1] \times [0, H_{\text{new}}-1]$ in the new lightmap:
1. $s_{\text{new}} = (\text{mins}_{\text{new}}[0] + x) \times 16$
2. $t_{\text{new}} = (\text{mins}_{\text{new}}[1] + y) \times 16$
3. Compute $\mathbf{P} = \mathbf{P}_{\text{world}}(s_{\text{new}}, t_{\text{new}})$.
4. Project $\mathbf{P}$ into source $(s, t)$ coordinates:
   $$s_{\text{orig}} = \mathbf{P} \cdot \mathbf{vS}_{\text{orig}} + \text{shiftS}_{\text{orig}}$$
   $$t_{\text{orig}} = \mathbf{P} \cdot \mathbf{vT}_{\text{orig}} + \text{shiftT}_{\text{orig}}$$
5. Convert to source continuous lightmap pixel indices:
   $$u_{\text{src}} = \frac{s_{\text{orig}} - \text{mins}_{\text{orig}}[0] \times 16}{16.0}$$
   $$v_{\text{src}} = \frac{t_{\text{orig}} - \text{mins}_{\text{orig}}[1] \times 16}{16.0}$$
6. Bilinearly sample the source lightmap bitmap at $(u_{\text{src}}, v_{\text{src}})$ with clamped boundary extension:
   $$u_0 = \text{clamp}(\lfloor u_{\text{src}} \rfloor, 0, W_{\text{orig}}-1), \quad u_1 = \text{clamp}(u_0 + 1, 0, W_{\text{orig}}-1)$$
   $$v_0 = \text{clamp}(\lfloor v_{\text{src}} \rfloor, 0, H_{\text{orig}}-1), \quad v_1 = \text{clamp}(v_0 + 1, 0, H_{\text{orig}}-1)$$
   $$\alpha = u_{\text{src}} - \lfloor u_{\text{src}} \rfloor, \quad \beta = v_{\text{src}} - \lfloor v_{\text{src}} \rfloor$$
   $$C = (1-\alpha)(1-\beta) C(u_0, v_0) + \alpha(1-\beta) C(u_1, v_0) + (1-\alpha)\beta C(u_0, v_1) + \alpha\beta C(u_1, v_1)$$

---

### 2. Viewport Lightmap Decoupling (`COMP-2` & `COMP-3`)

In `src/editor/BspRenderer.h`, extend `LightmapInfo` to store the exact projection vectors at the time the atlas was generated:

```cpp
struct LightmapInfo
{
    int w, h;
    int x[MAX_LIGHTMAPS];
    int y[MAX_LIGHTMAPS];
    int atlasId[MAX_LIGHTMAPS];
    
    // Projection basis when atlas was created
    vec3 vS;
    vec3 vT;
    float shiftS;
    float shiftT;
    int imins[2];
    int imaxs[2];
    float midTexU;
    float midTexV;
    float midPolyU;
    float midPolyV;
};
```

In `BspRenderer::updateFaceUVs` (`src/editor/BspRenderer.cpp`):
```cpp
// 1. Texture UVs use active texinfo (updates in real-time as user drags sliders)
float fU = dotProduct(texinfo.vS, pos) + texinfo.shiftS;
float fV = dotProduct(texinfo.vT, pos) + texinfo.shiftT;
vert.u = fU * tw;
vert.v = fV * th;

// 2. Lightmap UVs use FIXED LightmapInfo basis (remains perfectly stationary)
if (hasLighting && lmap && lmap->w > 0 && lmap->h > 0 && textureStep > 0)
{
    float fLightU = dotProduct(lmap->vS, pos) + lmap->shiftS;
    float fLightV = dotProduct(lmap->vT, pos) + lmap->shiftT;

    float fLightMapU = lmap->midTexU + (fLightU - lmap->midPolyU) / textureStep;
    float fLightMapV = lmap->midTexV + (fLightV - lmap->midPolyV) / textureStep;

    float uu = (fLightMapU / (float)lmap->w) * lw;
    float vv = (fLightMapV / (float)lmap->h) * lh;

    float pixelStep = 1.0f / (float)MAX_LIGHTMAP_ATLAS_SIZE;
    for (int s = 0; s < MAX_LIGHTMAPS; s++)
    {
        vert.luv[s][0] = uu + lmap->x[s] * pixelStep;
        vert.luv[s][1] = vv + lmap->y[s] * pixelStep;
    }
}
```

---

### 3. Baseline Lightmap Cache & Undo Integration (`COMP-4`)

To eliminate blur and corruption caused by successive edits:
1. When `undo_lightmaps` is saved or when a user begins face editing, store:
   - `std::vector<COLOR3> rawData`: Original unresampled lightmap colors.
   - `int width, height, layers`: Original dimensions.
   - `BSPTEXTUREINFO texinfo`: Original projection vectors.
   - `int imins[2], imaxs[2]`: Original extents.
2. `Bsp::resize_all_lightmaps()` checks each face against its baseline. If a face has been resized multiple times in a row without a full atlas reload, it resamples directly from the pristine baseline copy.

---

## Testing & Verification Plan

1. **Real-time Viewport Test**: Open a lighted BSP, select a face, and drag `Scale X/Y` from 0.1 to 10.0 and `Shift X/Y` continuously. Verify that surface texture scales/shifts smoothly while the lightmap shadows and highlights stay completely stationary on the geometry.
2. **In-Game Lighting Verification**: Modify face texture scales and shifts, save the BSP, launch Half-Life/engine, and verify:
   - No surface extent crashes.
   - No light bleeding across faces.
   - Lighting patterns match the original unedited compile.
3. **Multi-Face Batch Editing**: Select 10+ faces on different planes and adjust texture scale simultaneously. Verify all faces update and resample accurately.
4. **Copy/Paste Style Test**: Copy texture style from face A to face B. Verify face B receives new texture scale/shift without corrupting or shifting its baked lightmap.
