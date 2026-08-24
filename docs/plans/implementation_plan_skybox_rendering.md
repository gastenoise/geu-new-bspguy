# Implementation Plan: End-to-End Skybox Rendering

This implementation plan details the hierarchical task breakdown for developing, integrating, and verifying the 3D skybox rendering feature across `revamped-newbspguy`.

---

## Task Hierarchy & Checklists

- [x] 1. Settings, Configuration & UI Foundations
  - [x] 1.1 Add `RENDER_SKYBOX = 1 << 18` to `enum RenderFlags` in `src/editor/Settings.h`.
    - _Requirements: REQ-4.1_
    - _Dependencies: None_
  - [x] 1.2 Add `std::string skybox_dir` member to `Settings` struct in `src/editor/Settings.h` and default initialization in `src/editor/Settings.cpp`.
    - _Requirements: REQ-4.2_
    - _Dependencies: Task 1.1_
  - [x] 1.3 Implement INI loading and saving for `render_skybox` and `skybox_dir` in `src/editor/Settings.cpp`.
    - _Requirements: REQ-4.3_
    - _Dependencies: Task 1.2_
  - [x] 1.4 Add `"Render Skybox"` checkbox and `"Skybox Directory"` path input with `"Browse..."` file picker in `src/editor/gui/GuiDialogs.cpp` (Settings -> Render tab).
    - _Requirements: REQ-4.4_
    - _Dependencies: Task 1.3_
  - [x] 1.5 Add `"Skybox"` menu item under `View` in `src/editor/gui/GuiMenuBar.cpp`.
    - _Requirements: REQ-5.1_
    - _Dependencies: Task 1.1_
  - [x] 1.6 Register `"view.skybox_toggle"` action in `src/editor/gui/ActionRegistryInit.cpp` for Command Palette (`Ctrl+K`) and shortcuts.
    - _Requirements: REQ-5.2, REQ-5.3_
    - _Dependencies: Task 1.5_

- [x] 2. Skybox Asset Discovery & Image Loading Engine
  - [x] 2.1 Create `src/editor/Skybox.h` and `src/editor/Skybox.cpp` declaring the `Skybox` class with 6-face container and OpenGL texture handle.
    - _Requirements: REQ-1.5, REQ-2.1_
    - _Dependencies: Phase 1_
  - [x] 2.2 Implement multi-tier face path resolution `resolveFacePath()` in `Skybox.cpp` supporting custom directory, `<gamedir>/gfx/env/`, and `FindPathInAssets`.
    - _Requirements: REQ-1.3, REQ-1.4_
    - _Dependencies: Task 2.1_
  - [x] 2.3 Implement fallback resolution to `"desert"` skybox when map skyname is not found on disk, and fallback to 2D when missing entirely.
    - _Requirements: REQ-1.2, REQ-1.4, REQ-3.4_
    - _Dependencies: Task 2.2_
  - [x] 2.4 Implement image decoding using `stbi_load` (from `src/filedialog/stb_image.h`) supporting `.tga`, `.png`, `.bmp`, and `.jpg`.
    - _Requirements: REQ-2.1_
    - _Dependencies: Task 2.3_
  - [x] 2.5 Implement `Skybox::upload()` creating and configuring `GL_TEXTURE_CUBE_MAP` with `GL_CLAMP_TO_EDGE` and `GL_LINEAR` filtering.
    - _Requirements: REQ-2.2, REQ-2.3, REQ-2.4_
    - _Dependencies: Task 2.4_
  - [x] 2.6 Implement `Skybox::clear()` and destructor to properly release CPU buffers and delete OpenGL cubemap texture IDs.
    - _Requirements: REQ-6.1_
    - _Dependencies: Task 2.5_

- [x] 3. GLSL Skybox Shader & Rendering Pipeline
  - [x] 3.1 Define `g_shader_skybox_vertex` and `g_shader_skybox_fragment` in `src/gl/shaders.h` and `src/gl/shaders.cpp`.
    - _Requirements: REQ-3.1, REQ-3.2_
    - _Dependencies: Phase 2_
  - [x] 3.2 Add `ShaderProgram* skyShader` in `src/editor/Renderer.h` and compile/initialize uniforms (`modelViewProjection`, `uCameraOrigin`, `sSkybox`) in `src/editor/Renderer.cpp`.
    - _Requirements: REQ-3.1_
    - _Dependencies: Task 3.1_
  - [x] 3.3 Add `Skybox* skybox` instance to `src/editor/BspRenderer.h`, instantiate it in constructor, and clean it up in destructor.
    - _Requirements: REQ-1.1, REQ-6.1_
    - _Dependencies: Phase 2_
  - [x] 3.4 Integrate `skybox->load()` into asynchronous texture loading (`BspRenderer::loadTextures()`) and upload on the GL thread (`BspRenderer::reuploadTextures()`).
    - _Requirements: REQ-2.5_
    - _Dependencies: Task 3.3_
  - [x] 3.5 Update `BspRenderer::drawModel()` in `src/editor/BspRenderer.cpp` to bind `skyShader` and cubemap when rendering sky render groups with `RENDER_SKYBOX` enabled.
    - _Requirements: REQ-3.3, REQ-3.4_
    - _Dependencies: Task 3.2, Task 3.4_
  - [x] 3.6 Ensure compatibility with screenshot capture FBOs and map overview mode.
    - _Requirements: REQ-3.5_
    - _Dependencies: Task 3.5_

- [x] 4. Build Verification & Quality Assurance
  - [x] 4.1 Compile the entire codebase with CMake and verify zero compiler warnings or linking errors.
    - _Requirements: REQ-6.1_
    - _Dependencies: Phase 3_
  - [x] 4.2 Test with standard GoldSrc maps (`cstrike`, `valve`) verifying orientation, horizon alignment, and absence of edge seams.
    - _Requirements: REQ-3.3, REQ-6.2_
    - _Dependencies: Task 4.1_
  - [x] 4.3 Test edge cases: missing skyname (desert fallback), invalid skybox path (2D fallback), runtime toggle on/off, and map reload.
    - _Requirements: REQ-1.2, REQ-3.4, REQ-6.3_
    - _Dependencies: Task 4.2_
