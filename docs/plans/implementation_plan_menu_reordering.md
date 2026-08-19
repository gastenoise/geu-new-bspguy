# Implementation Plan: Modularization & Menu Reordering

## Project Boundaries

### Must-Have (In-Scope)
- [ ] Create directory structure `src/editor/gui/`.
- [ ] Split `Gui.cpp` into modular files: `GuiMenuBar.cpp`, `GuiContextMenu.cpp`, `GuiWidgets.cpp`, `GuiDialogs.cpp`.
- [ ] Update `CMakeLists.txt` to include new modular GUI source files.
- [ ] Implement new 8-category menu structure (**File, Edit, View, Map, Tools, Create, Windows, Help**).
- [ ] Group `Save As...` formats into clear engine target categories.
- [ ] Consolidate repair, hull, and cull box tools into the `Tools` menu.
- [ ] Isolate experimental/WIP features under `Tools -> Experimental / WIP Tools`.
- [ ] Refactor viewport context menus into `GuiContextMenu.cpp`.
- [ ] Maintain 100% feature parity and ensure clean compilation on Windows (MSVC) and Linux (GCC/Clang).

---

## Phased Implementation Plan

- [ ] 1. **Phase 1: Modular Infrastructure & CMake Setup**
  - [ ] 1.1 Create directory `src/editor/gui/`.
    - _Requirements: REQ-1_
    - _Dependencies: None_
  - [ ] 1.2 Declare modular helper methods in `src/editor/Gui.h`.
    - _Requirements: REQ-1_
    - _Dependencies: Task 1.1_
  - [ ] 1.3 Add new `.cpp` files to `SOURCE_FILES` in `CMakeLists.txt`.
    - _Requirements: REQ-1_
    - _Dependencies: Task 1.1_
  - [ ] 1.4 Add new localization key constants in `resources/languages/language.ini`.
    - _Requirements: REQ-1, REQ-5, REQ-7_
    - _Dependencies: None_

- [ ] 2. **Phase 2: Extract & Implement Main Menu Bar (`GuiMenuBar.cpp`)**
  - [ ] 2.1 Create `src/editor/gui/GuiMenuBar.cpp` and implement `Gui::drawMenuBar()`.
    - _Requirements: REQ-1, REQ-2_
    - _Dependencies: Phase 1_
  - [ ] 2.2 Implement `drawMenu_File()` with grouped `Save As...`, `Export`, `Import` submenus.
    - _Requirements: REQ-3_
    - _Dependencies: Task 2.1_
  - [ ] 2.3 Implement `drawMenu_Edit()` with consolidated clipboard and transformation items.
    - _Requirements: REQ-4_
    - _Dependencies: Task 2.1_
  - [ ] 2.4 Implement `drawMenu_View()` with clipnode render hull selector and panel toggles.
    - _Requirements: REQ-5_
    - _Dependencies: Task 2.1_
  - [ ] 2.5 Implement `drawMenu_Map()` with streamlined map inspection and transformation actions.
    - _Requirements: REQ-6_
    - _Dependencies: Task 2.1_
  - [ ] 2.6 Implement `drawMenu_Tools()` with Hull Management, Cull Operations, Geometry Fixes, and WIP Tools.
    - _Requirements: REQ-7_
    - _Dependencies: Task 2.1_
  - [ ] 2.7 Implement `drawMenu_Create()`, `drawMenu_Windows()`, `drawMenu_Help()`, and `drawMenu_Debug()`.
    - _Requirements: REQ-8_
    - _Dependencies: Task 2.1_

- [ ] 3. **Phase 3: Extract & Implement Context Menus (`GuiContextMenu.cpp`)**
  - [ ] 3.1 Create `src/editor/gui/GuiContextMenu.cpp` and implement `Gui::drawBspContexMenu()`.
    - _Requirements: REQ-1, REQ-9_
    - _Dependencies: Phase 1_
  - [ ] 3.2 Implement `drawContextMenu_Entity()`, `drawContextMenu_Face()`, and `drawContextMenu_Empty()`.
    - _Requirements: REQ-9_
    - _Dependencies: Task 3.1_

- [ ] 4. **Phase 4: Extract Widgets & Dialogs (`GuiWidgets.cpp` & `GuiDialogs.cpp`)**
  - [ ] 4.1 Create `src/editor/gui/GuiWidgets.cpp` and extract floating editor panels (`KeyvalueEditor`, `TextureBrowser`, `TransformWidget`, `LightMapTool`, etc.).
    - _Requirements: REQ-1_
    - _Dependencies: Phase 1_
  - [ ] 4.2 Create `src/editor/gui/GuiDialogs.cpp` and extract modal windows (`Settings`, `About`, `Help`, `MergeWindow`, `ImportMapWidget`).
    - _Requirements: REQ-1_
    - _Dependencies: Phase 1_

- [ ] 5. **Phase 5: Build Verification & Regression Testing**
  - [ ] 5.1 Perform CMake build on MSVC to verify zero compilation or linkage errors.
    - _Requirements: REQ-1 through REQ-9_
    - _Dependencies: Phases 1-4_
  - [ ] 5.2 Execute manual UI smoke tests to verify every menu item, dialog, and context popup functions correctly.
    - _Requirements: REQ-1 through REQ-9_
    - _Dependencies: Task 5.1_
