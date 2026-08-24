# Design Document: End-to-End Skybox Rendering Architecture

## Overview

This document specifies the technical design and architectural components for implementing full 3D skybox rendering in `revamped-newbspguy`. The implementation introduces a dedicated `Skybox` management module, a view-ray GLSL projection shader, a multi-tier asset resolution engine, and unified UI and settings integration.

---

## System Architecture

### Component Map

| Component ID | Name | Subsystem / File | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-1** | `Skybox` | `src/editor/Skybox.h`, `Skybox.cpp` | Manages 6-face texture lifecycle, GL Cubemap / 2D texture resources, image decoding, and asset discovery | `BspRenderer`, `Renderer`, `Settings` |
| **COMP-2** | `SkyboxShader` | `src/gl/shaders.h`, `shaders.cpp` | GLSL vertex & fragment shader computing view-ray vectors and sampling the skybox | `ShaderProgram`, `Renderer`, `BspRenderer` |
| **COMP-3** | `BspRenderer` Sky Extension | `src/editor/BspRenderer.h`, `BspRenderer.cpp` | Integrates skybox loading into texture streaming pipeline; binds sky shader for `isSky` render groups | `Skybox`, `ShaderProgram`, `Bsp` |
| **COMP-4** | Settings & Persistence | `src/editor/Settings.h`, `Settings.cpp` | Manages `RENDER_SKYBOX` flag and `skybox_dir` configuration in `settings.ini` | `GuiDialogs`, `BspRenderer` |
| **COMP-5** | UI & Action Registry | `src/editor/gui/GuiMenuBar.cpp`, `GuiDialogs.cpp`, `ActionRegistryInit.cpp` | Provides menu items, settings dialog inputs, folder picker, and Command Palette shortcuts | `Settings`, `ActionRegistry`, `ImFileDialog` |

### High-Level Architecture Diagram

```
+-------------------------------------------------------------------------------+
|                               GUI / Settings Layer                            |
|  - View Menu ("Skybox")                                                       |
|  - Settings Dialog (Render Tab -> Checkbox & Skybox Directory Input + Browse) |
|  - ActionRegistry ("view.skybox_toggle")                                      |
+---------------------------------------+---------------------------------------+
                                        |
                                        v
+-------------------------------------------------------------------------------+
|                       Settings & Global State (COMP-4)                        |
|  - g_render_flags (RENDER_SKYBOX)                                             |
|  - g_settings.skybox_dir                                                      |
+---------------------------------------+---------------------------------------+
                                        |
                                        v
+-------------------------------------------------------------------------------+
|                         BspRenderer Subsystem (COMP-3)                        |
|  1. Reads worldspawn "skyname" (defaults to "desert")                         |
|  2. Calls Skybox::load() asynchronously in texturesFuture                     |
|  3. Separates sky faces into dedicated render groups                          |
|  4. In drawModel(): binds SkyboxShader & Skybox Cubemap for sky groups        |
+-------------------+---------------------------------------+-------------------+
                    |                                       |
                    v                                       v
+-----------------------------------+   +---------------------------------------+
|        Skybox Module (COMP-1)     |   |         GLSL Shaders (COMP-2)         |
|  - Multi-tier asset resolution    |   |  - g_shader_skybox_vertex             |
|  - stbi_load (.tga, .png, etc.)   |   |  - g_shader_skybox_fragment           |
|  - GL_TEXTURE_CUBE_MAP generation |   |  - vRayDir = worldPos - cameraOrigin  |
|  - GL_CLAMP_TO_EDGE configuration |   |  - Fragment samples textureCube()     |
+-----------------------------------+   +---------------------------------------+
```

---

## Data Flow Specifications

### 1. Skybox Discovery & Asset Loading Flow

```
1. Map Load/Reload -> BspRenderer reads map->ents[0]->keyvalues["skyname"]
2. IF skyname is empty -> skyname = "desert"
3. Skybox::load(map, skyname, configuredSkyboxDir) initiated in background thread:
   a. Check configuredSkyboxDir / <skyname>[up|dn|ft|bk|lf|rt].tga/.png
   b. Check g_game_dir + "gfx/env/" + <skyname>*.tga/.png
   c. Check FindPathInAssets(map, "gfx/env/" + <skyname>*.tga)
   d. IF not found and skyname != "desert": retry steps a-c with skyname = "desert"
   e. IF all 6 sides found: decode via stbi_load -> create GL_TEXTURE_CUBE_MAP
   f. IF failed: mark skybox unavailable -> fallback to skyTex_rgba (2D flat)
```

### 2. Real-Time Viewport Rendering Flow

```
1. Frame render in BspRenderer::render():
2. In drawModel():
   IF rgroup is sky AND (g_render_flags & RENDER_SKYBOX) AND skybox->isLoaded():
      a. Bind skyShader
      b. Upload modelViewProjection matrix
      c. Set uniform vec3 uCameraOrigin = localCameraOrigin
      d. Bind skybox GL_TEXTURE_CUBE_MAP to texture unit 0
      e. Draw sky vertex buffer
   ELSE:
      a. Render using default bspShader with skyTex_rgba
```

---

## Components and Interfaces

### 1. `Skybox` Class (`src/editor/Skybox.h`, `src/editor/Skybox.cpp`)

```cpp
#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>

class Bsp;

enum SkyboxSide : int
{
    SKYBOX_RIGHT = 0, // rt (+X in Cubemap / -Y in GoldSrc)
    SKYBOX_LEFT  = 1, // lf (-X in Cubemap / +Y in GoldSrc)
    SKYBOX_BACK  = 2, // bk (+Z in Cubemap / -X in GoldSrc)
    SKYBOX_FRONT = 3, // ft (-Z in Cubemap / +X in GoldSrc)
    SKYBOX_UP    = 4, // up (+Y in Cubemap / +Z in GoldSrc)
    SKYBOX_DOWN  = 5, // dn (-Y in Cubemap / -Z in GoldSrc)
    SKYBOX_COUNT = 6
};

class Skybox
{
public:
    std::string skyname;
    GLuint cubemapTexId;
    bool loaded;
    int width, height;

    Skybox();
    ~Skybox();

    // Locates and loads the 6 images, preparing raw pixel buffers
    bool load(Bsp* map, const std::string& skyName, const std::string& customDir);

    // Uploads decoded pixel buffers to OpenGL cubemap (must run on main GL thread)
    void upload();

    // Releases GPU and CPU memory
    void clear();

    // Binds the cubemap texture to the given texture unit
    void bind(GLuint unit = 0);

    bool isLoaded() const { return loaded && cubemapTexId != 0; }

private:
    struct FaceImage
    {
        unsigned char* data;
        int w, h, channels;
        std::string filePath;
    };

    FaceImage faces[SKYBOX_COUNT];
    bool resolveFacePath(Bsp* map, const std::string& prefix, const std::string& suffix, 
                         const std::string& customDir, std::string& outPath);
};
```

### 2. GLSL Skybox Shader (`src/gl/shaders.h`, `src/gl/shaders.cpp`)

#### Vertex Shader (`g_shader_skybox_vertex`):
```glsl
uniform mat4 modelViewProjection;
uniform vec3 uCameraOrigin;

attribute vec3 vPosition;

varying vec3 fRayDir;

void main()
{
    gl_Position = modelViewProjection * vec4(vPosition, 1.0);
    // In bspguy OpenGL coordinates: position is already in flipped GL space
    // Ray direction is vector from camera origin (in GL coords) to vertex
    fRayDir = vPosition - uCameraOrigin;
}
```

#### Fragment Shader (`g_shader_skybox_fragment`):
```glsl
varying vec3 fRayDir;

uniform samplerCube sSkybox;

void main()
{
    // Vector pointing into cubemap
    vec3 dir = normalize(fRayDir);
    
    // Sample cubemap
    vec4 skyColor = textureCube(sSkybox, dir);
    
    float gamma = 1.6;
    gl_FragColor = vec4(pow(skyColor.rgb, vec3(1.0 / gamma)), 1.0);
}
```

### 3. Cubemap Coordinate Alignment (GoldSrc to OpenGL Cubemap)

GoldSrc coordinates: `+X` = Forward, `+Y` = Left, `+Z` = Up.
bspguy OpenGL coordinates: `vPosition = vec3(x, z, -y)`.

Mapping GoldSrc 6 sky files to standard OpenGL Cubemap targets (`GL_TEXTURE_CUBE_MAP_POSITIVE_X`, etc.):
- `GL_TEXTURE_CUBE_MAP_POSITIVE_X` (+X in GL) -> `ft` (Front / +X in HL)
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_X` (-X in GL) -> `bk` (Back / -X in HL)
- `GL_TEXTURE_CUBE_MAP_POSITIVE_Y` (+Y in GL) -> `up` (Top / +Z in HL)
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_Y` (-Y in GL) -> `dn` (Bottom / -Z in HL)
- `GL_TEXTURE_CUBE_MAP_POSITIVE_Z` (+Z in GL) -> `lf` (Left / +Y in HL)
- `GL_TEXTURE_CUBE_MAP_NEGATIVE_Z` (-Z in GL) -> `rt` (Right / -Y in HL)

Images loaded into `GL_TEXTURE_CUBE_MAP` will be uploaded with correct orientation to ensure seamless sky alignment with map orientation.

---

## Error Handling & Fallback Strategy

| Failure Scenario | Recovery Behavior |
| :--- | :--- |
| Map has no `skyname` key in `worldspawn` | Default to `"desert"` skybox and attempt standard resolution. |
| Specified `skyname` files not found on disk | Attempt fallback to `"desert"` skybox in `<gamedir>/gfx/env/` or asset paths. |
| Neither `skyname` nor `"desert"` found | Disable skybox rendering for current map and fall back to 2D `skyTex_rgba`. |
| Corrupt or invalid image format on one side | Log error with missing side name and fall back to 2D `skyTex_rgba`. |
| User disables `RENDER_SKYBOX` toggle | Immediate runtime switch to 2D planar rendering without reloading textures. |

---

## Testing Strategy

1. **Unit & Asset Resolution Tests**:
   - Verify `resolveFacePath()` finds `.tga`, `.png`, and `.bmp` files in configured path, game dir, and asset paths.
   - Verify `"desert"` fallback triggers when map uses non-existent skyname.
2. **Visual & Alignment Tests**:
   - Verify skybox horizon alignment on standard GoldSrc maps (e.g. `de_dust2` / `"desert"`, `cs_assault` / `"city"`, `de_nuke` / `"cliff"`).
   - Check all 4 horizon sides and top/bottom seams for pixel-perfect continuity.
3. **Interactive & Performance Tests**:
   - Toggle `RENDER_SKYBOX` via Menu, Settings, and Command Palette during real-time navigation.
   - Test map switching, reloading, and overview screenshot exports.
