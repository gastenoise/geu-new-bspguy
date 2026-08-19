# Implementation Plan: Menu Reordering & Refactoring

## Project Boundaries

### Must-Have (In-Scope)
- [ ] Refactor `Gui::drawMenuBar()` into 8 modular helper methods in `Gui.cpp`.
- [ ] Implement the new menu order: **File, Edit, View, Map, Tools, Create, Windows, Help**.
- [ ] Group `Save As...` formats into clear engine target categories.
- [ ] Consolidate all repair, hull, and cull box tools into the `Tools` menu.
- [ ] Isolate all experimental/WIP features under `Tools -> Experimental / WIP Tools`.
- [ ] Refactor viewport context menus (`ent_context`, `face_context`, `empty_context`).
- [ ] Ensure 100% feature parity (no lost functionality or broken shortcuts).

### Out-Of-Scope
- [ ] Rewriting engine rendering logic or BSP compilation logic.
- [ ] Modifying third-party ImGui or GLFW backends.

---

## Phased Implementation Plan

- [ ] 1. **Phase 1: Architecture & Header Preparation**
  - [ ] 1.1 Add modular menu helper function declarations to `src/editor/Gui.h`.
    - _Requirements: REQ-1_
    - _Dependencies: None_
  - [ ] 1.2 Define new localization key constants in `src/util/lang_defs.h` and `resources/languages/language.ini`.
    - _Requirements: REQ-1, REQ-4, REQ-6_
    - _Dependencies: Task 1.1_

- [ ] 2. **Phase 2: File & Edit Menu Refactoring**
  - [ ] 2.1 Implement `Gui::drawMenu_File()` in `Gui.cpp` (Save presets, Export/Import submenus, Sven Test, Settings, Exit).
    - _Requirements: REQ-2_
    - _Dependencies: Phase 1_
  - [ ] 2.2 Implement `Gui::drawMenu_Edit()` in `Gui.cpp` (History, Clipboard, Duplicate, Alignment, Selection).
    - _Requirements: REQ-3_
    - _Dependencies: Phase 1_

- [ ] 3. **Phase 3: View & Map Menu Refactoring**
  - [ ] 3.1 Implement `Gui::drawMenu_View()` in `Gui.cpp` (Clipnodes render hull, Widget panel toggles, GoTo, Limits).
    - _Requirements: REQ-4_
    - _Dependencies: Phase 1_
  - [ ] 3.2 Implement `Gui::drawMenu_Map()` in `Gui.cpp` (Entity Report, Limits check, Clean, Optimize, Transforms, RAD).
    - _Requirements: REQ-5_
    - _Dependencies: Phase 1_

- [ ] 4. **Phase 4: Tools & Create Menu Implementation**
  - [ ] 4.1 Implement `Gui::drawMenu_Tools()` in `Gui.cpp` (Hull Management, Cull Area Operations, Geometry Repair, OOB, WIP Tools).
    - _Requirements: REQ-6_
    - _Dependencies: Phase 1_
  - [ ] 4.2 Implement `Gui::drawMenu_Create()` in `Gui.cpp` (Entities, Brush models, Prefabs, Skybox).
    - _Requirements: REQ-7_
    - _Dependencies: Phase 1_

- [ ] 5. **Phase 5: Windows, Help & Context Menu Refactoring**
  - [ ] 5.1 Implement `Gui::drawMenu_Windows()` and `Gui::drawMenu_Help()` in `Gui.cpp`.
    - _Requirements: REQ-1_
    - _Dependencies: Phase 1_
  - [ ] 5.2 Refactor viewport context menus (`drawContextMenu_Entity`, `drawContextMenu_Face`, `drawContextMenu_Empty`).
    - _Requirements: REQ-8_
    - _Dependencies: Phase 1_

- [ ] 6. **Phase 6: Verification & Compilation Check**
  - [ ] 6.1 Build solution with CMake / MSVC / GCC to verify clean compilation without warnings.
    - _Requirements: REQ-1 through REQ-8_
    - _Dependencies: Phases 1-5_
  - [ ] 6.2 Conduct manual functional smoke testing of all menu entries.
    - _Requirements: REQ-1 through REQ-8_
    - _Dependencies: Task 6.1_
