# Design Document: GoldSrc-Accurate Decal Rendering and Interaction

## Overview

This document specifies the technical design and architecture for GoldSrc-accurate decal rendering and interaction in `revamped-newbspguy`. 

When the decal rendering option is enabled, `infodecal` entities are displayed on their host BSP surfaces as unscaled, alpha-blended planar textures using their native dimensions, the palette rule specific to GoldSrc (where `palette[255]` defines the world color and pixel grayscale luminosity dictates opacity), and convex polygon clipping against host face boundaries.

Additionally, decal lighting mode is configurable between **Fullbright (Unlit)** for standard editor visibility and **Lightmap-Modulated** for in-game lighting accuracy. Decal selection via viewport raycasting performs an accurate planar hit test on the decal's surface.

---

## Architecture and Components

### Component Map

| Component ID | Module / File | Responsibility | Interfaces With |
|---|---|---|---|
| **COMP-1** | `src/res/Wad.*`, `src/util/util.*` | Decal texture parsing, palette extraction, and RGBA conversion (`ConvertDecalWadTexToRGBA`) | `Wad`, `Texture`, OpenGL |
| **COMP-2** | `src/editor/BspRenderer.*` | Decal projection math, Sutherland-Hodgman polygon clipping, GPU buffer creation, rendering passes | `Bsp`, `RenderEnt`, `VertexBuffer`, `Shaders` |
| **COMP-3** | `src/editor/BspRenderer::pickPoly` | Planar raycast intersection and viewport selection handling for decals | `PickInfo`, Camera, `FaceMath` |
| **COMP-4** | `src/editor/Settings.*`, `src/editor/gui/*` | Render flag `RENDER_DECALS`, `decal_lighting_mode`, UI controls (View menu, Settings dialog), command palette actions, and i18n | `Gui`, `ActionRegistry`, `imgui` |

```
┌────────────────────────────────────────────────────────┐
│                      revamped-newbspguy                │
│                                                        │
│   ┌────────────────┐      ┌────────────────────────┐   │
│   │   WAD Loader   │ ---> │ Decal RGBA Decoder     │   │
│   │  (decals.wad)  │      │ (palette[255] + Alpha) │   │
│   └────────────────┘      └───────────┬────────────┘   │
│                                       │ (Texture*)     │
│   ┌────────────────┐                  v                │
│   │  infodecal Ent │ ---> ┌────────────────────────┐   │
│   │ (origin, tex)  │      │ Decal Projector        │   │
│   └────────────────┘      │ (Find Face & Clip Quad)│   │
│                           └───────────┬────────────┘   │
│                                       │ (Decal Mesh)   │
│           ┌───────────────────────────┴──────────┐     │
│           v                                      v     │
│   ┌────────────────┐                    ┌────────────┐ │
│   │ Decal Renderer │                    │ Raycast    │ │
│   │ (Fullbright /  │                    │ Picking    │ │
│   │  Lightmapped)  │                    │            │ │
│   └────────────────┘                    └────────────┘ │
└────────────────────────────────────────────────────────┘
```

---

## Data Structures

### 1. `DecalRenderData`
Stored per `RenderEnt` for entities representing decals:

```cpp
struct DecalRenderData
{
    Texture* texture = nullptr;              // Uploaded OpenGL decal texture (RGBA)
    VertexBuffer* vertexBuffer = nullptr;    // Textured polygon vertices (GL_TRIANGLES)
    VertexBuffer* wireframeBuffer = nullptr; // Selection wireframe outline
    vec3 center;                             // Projected center on face plane
    vec3 planeNormal;                        // Normal of host face
    vec3 sDir;                               // Horizontal texture direction on surface
    vec3 tDir;                               // Vertical texture direction on surface
    float width = 0.0f;                      // Native width in world units (texels)
    float height = 0.0f;                     // Native height in world units (texels)
    int hostFaceIdx = -1;                    // Index of host face in map (-1 if orphan)
    std::vector<vec3> worldVerts;            // World-space vertices of clipped decal polygon
    bool valid = false;                      // True if properly projected onto a surface

    DecalRenderData();
    ~DecalRenderData();
    void clean();
};
```

### 2. Integration into `RenderEnt` (`src/editor/BspRenderer.h`)

```cpp
struct RenderEnt
{
    // ... existing fields ...
    DecalRenderData* decal = nullptr;
    // ...
};
```

---

## Detailed Component Specifications

### 1. Decal Texture Processing & RGBA Conversion (`COMP-1`)

GoldSrc decal textures in WAD3 files are 8-bit indexed images where:
- `palette[255]` is the decal's designated world RGB color.
- Each pixel's index maps to a grayscale color in `palette[index]`.
- Brightness $L = \text{palette}[\text{index}].r$.
- Alpha $\text{Alpha} = 255 - L$. White ($L = 255$) yields $\text{Alpha} = 0$; Black ($L = 0$) yields $\text{Alpha} = 255$.

```cpp
COLOR4* ConvertDecalWadTexToRGBA(const WADTEX& wadTex)
{
    int lastMipSize = (wadTex.nWidth >> 3) * (wadTex.nHeight >> 3);
    const unsigned char* src = wadTex.data.data();
    const COLOR3* palette = (const COLOR3*)(src + wadTex.nOffsets[3] + lastMipSize + sizeof(short) - sizeof(BSPMIPTEX));

    COLOR3 decalColor = palette[255];
    int sz = wadTex.nWidth * wadTex.nHeight;
    COLOR4* imageData = new COLOR4[sz];

    for (int k = 0; k < sz; k++)
    {
        unsigned char lum = palette[src[k]].r;
        unsigned char alpha = (lum == 255) ? 0 : (255 - lum);
        imageData[k] = COLOR4(decalColor.r, decalColor.g, decalColor.b, alpha);
    }
    return imageData;
}
```

Uploaded to OpenGL with:
- `Texture::TYPE_DECAL`
- `GL_TEXTURE_WRAP_S = GL_CLAMP_TO_EDGE`
- `GL_TEXTURE_WRAP_T = GL_CLAMP_TO_EDGE`
- `GL_TEXTURE_MIN_FILTER = GL_LINEAR`
- `GL_TEXTURE_MAG_FILTER = GL_LINEAR`

A cache mapping `texName -> Texture*` ensures textures shared by multiple decals are uploaded once.

---

### 2. Decal Surface Search, Projection & Polygon Clipping (`COMP-2`)

#### A. Host Face Search
Given an `infodecal` entity at position $\vec{O} = \text{ent->origin}$:
1. Find candidate faces in `faceMaths` where plane distance $d = | \vec{N} \cdot \vec{O} - \text{DistPlane} | \le 8.0\text{ units}$.
2. Project $\vec{O}$ onto the candidate plane: $P = \vec{O} - \vec{N}(\vec{N} \cdot \vec{O} - \text{DistPlane})$.
3. Check proximity to the face polygon using `faceMath.worldToLocal` and bounding extents.
4. Select the face with minimal $d$.

#### B. Texture Coordinate Orientation
From the selected face's `texinfo`:
$$\vec{S}_{\text{dir}} = \text{normalize}(\text{texinfo.vS}), \quad \vec{T}_{\text{dir}} = \text{normalize}(\text{texinfo.vT})$$
If $\vec{S}_{\text{dir}}$ or $\vec{T}_{\text{dir}}$ is degenerate:
$$\text{TextureAxisFromPlane}(\vec{N}, \vec{S}_{\text{dir}}, \vec{T}_{\text{dir}})$$

#### C. Quad Construction & Polygon Clipping
Let $hw = W / 2.0$, $hh = H / 2.0$.
Initial 4 quad corners on face plane:
$$C_0 = P - \vec{S}_{\text{dir}} \cdot hw - \vec{T}_{\text{dir}} \cdot hh$$
$$C_1 = P + \vec{S}_{\text{dir}} \cdot hw - \vec{T}_{\text{dir}} \cdot hh$$
$$C_2 = P + \vec{S}_{\text{dir}} \cdot hw + \vec{T}_{\text{dir}} \cdot hh$$
$$C_3 = P - \vec{S}_{\text{dir}} \cdot hw + \vec{T}_{\text{dir}} \cdot hh$$

**Sutherland-Hodgman Convex Polygon Clipping:**
Clip the quad polygon against each edge of the host face polygon.
For every resulting clipped vertex $V_i$:
$$U(V_i) = \frac{(V_i - P) \cdot \vec{S}_{\text{dir}}}{W} + 0.5$$
$$V(V_i) = \frac{(V_i - P) \cdot \vec{T}_{\text{dir}}}{H} + 0.5$$

Build GPU vertex buffers (`tVert` with pos + texcoord) triangulating the clipped polygon.

---

### 3. Decal Rendering Pipeline & Lighting Modes (`COMP-2`)

In `BspRenderer::drawPointEntities` (or a dedicated decal render routine):

```cpp
if (g_render_flags & RENDER_DECALS)
{
    if (rendEntity.decal && rendEntity.decal->valid)
    {
        if (pass == REND_PASS_MODELSHADER)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1.0f, -1.0f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (g_settings.decal_lighting_mode == DECAL_LIGHTING_MODULATED && rendEntity.decal->hostFaceIdx >= 0)
            {
                // Modulate with host face light level / lightmap average
            }

            rendEntity.decal->texture->bind(0);
            rendEntity.decal->vertexBuffer->drawFull();

            glDisable(GL_POLYGON_OFFSET_FILL);
        }
        else if (pass == REND_PASS_COLORSHADER)
        {
            if (isSelected)
            {
                rendEntity.decal->wireframeBuffer->drawFull(); // Yellow selection outline
                rendEntity.pointEntCube->axesBuffer->drawFull(); // Center axes
            }
        }
        return; // Skip cube rendering
    }
}
// Fallback: render traditional pointEntCube
```

---

### 4. Raycast Picking & Selection (`COMP-3`)

In `BspRenderer::pickPoly`:

When testing entity $i$ with `g_render_flags & RENDER_DECALS`:
1. If `rendEntity.decal && rendEntity.decal->valid`:
   - Calculate ray-plane intersection with host plane ($\vec{N}, \text{fdist}$):
     $$t = \frac{\text{fdist} - \vec{N} \cdot \vec{\text{rayStart}}}{\vec{N} \cdot \vec{\text{rayDir}}}$$
   - If $t > 0$ and $t < \text{tempPickInfo.bestDist}$:
     - Hit position: $H = \vec{\text{rayStart}} + \vec{\text{rayDir}} \cdot t$.
     - Test if $H$ is inside the clipped decal polygon.
     - If inside:
       - Record hit: `tempPickInfo.SetSelectedEnt(i); tempPickInfo.bestDist = t; foundBetterPick = true;`
2. If decal is orphan or `RENDER_DECALS` is disabled:
   - Perform standard `pickAABB` against `pointEntCube`.

---

## Settings and User Interface (`COMP-4`)

1. **Flag Definition:** `RENDER_DECALS = 1 << 19` in `enum RenderFlags` (`Settings.h`).
2. **Lighting Setting:** `int decal_lighting_mode` (`0 = Fullbright, 1 = Lightmapped`) in `struct Settings`.
3. **Settings Dialog:** Added to `Settings -> 3D View` in `GuiDialogs.cpp` (checkbox for Render Decals and Combo / Radio for Lighting Mode).
4. **View Menu:** Added checkbox entry in `GuiMenuBar.cpp` under `View` menu.
5. **Action Registry:** Registered `"view.decals_toggle"` and `"view.decals_lighting_toggle"` in `ActionRegistryInit.cpp`.
6. **Localization:** Added string tokens in `lang_defs.h` and `.ini` files.

---

## Error Handling & Fallback Behavior

| Condition | Editor Behavior |
|---|---|
| Decal texture not found in any WAD | Fall back to standard purple point entity cube |
| No adjacent BSP face found ($> 8$ units) | Fall back to standard point entity cube |
| Non-conforming decal texture format | Fall back to standard point entity cube with log warning |
| Map unloaded / reloaded | Free GPU buffers and decal texture cache cleanly |
