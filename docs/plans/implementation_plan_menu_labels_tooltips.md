# Implementation Plan: Menu Labels Sanitization & Universal Tooltip Coverage

## Project Boundaries & Prerequisites
- **Branch**: `feat/menu-labels-and-tooltips`
- **Must Have**:
  - Delete `menu_original.md` and `menu_reordered.md` from repository root.
  - Sanitize `LANG_1095` across EN/RU/ZH language catalogs.
  - Fix typos (`"intenal"` -> `"internal"`).
  - Clean context menu shortcut labels (`"ALT+G"`, `"Ctrl+V"`).
  - Add hover tooltips (`IMGUI_TOOLTIP`) to all 241 menu bar items and 84 context menu items.
  - Successful Release build verification with CMake & MSVC.
- **Out of Scope**:
  - Restructuring menu hierarchies (already organized in 8 standard categories).
  - Modifying underlying BSP compilation or rendering algorithms.

---

## Phase 1: Repository Root Markdown Cleanup

- [x] 1.1 Remove Obsolete Menu Planning Markdown Files
  - Delete `menu_original.md` from repository root.
  - Delete `menu_reordered.md` from repository root.
  - Verify that `README.md` and all files under `docs/` are preserved.
  - _Requirements: REQ-1_

---

## Phase 2: Localization & String Catalogs Sanitization

- [x] 2.1 Sanitize `LANG_1095` in All Language Catalogs
  - In `resources/languages/language.ini`: Update `LANG_1095 = Go to...`
  - In `resources/languages/language_ru.ini`: Update `LANG_1095 = Перейти к...`
  - In `resources/languages/language_zh.ini`: Update `LANG_1095 = 转到...`
  - _Requirements: REQ-2.1_

- [x] 2.2 Fix Spelling Typos in Translation Catalogs
  - Replace `"intenal"` with `"internal"` in keys `LANG_0469`, `LANG_0470`, `LANG_1071`, `LANG_1072`, `LANG_1155`, `LANG_1156`, `LANG_1174`, `LANG_1175` across `language.ini`, `language_ru.ini`, and `language_zh.ini`.
  - _Requirements: REQ-2.3_

- [x] 2.3 Add Missing Localization Keys for Unlocalized Strings
  - Add language keys for hardcoded English menu titles and tooltips if required.
  - _Requirements: REQ-2_

---

## Phase 3: Main Menu Bar Refactoring & Universal Tooltip Coverage (`GuiMenuBar.cpp`)

- [x] 3.1 Refactor `drawMenu_File()`
  - Pass explicit shortcut strings to `ImGui::MenuItem` (e.g. `"Ctrl+S"`, `"Ctrl+O"`, `"Ctrl+W"`, `"F5"`, `"Ctrl+R"`, `"Ctrl+P"`, `"Alt+F4"`).
  - Add `IMGUI_TOOLTIP` to all Save, Save As presets, Open, Close, Export, Import, Sven Test, Merge, Recent Files, Reload, Validate, Settings, and Exit options.
  - _Requirements: REQ-3, REQ-5_

- [x] 3.2 Refactor `drawMenu_Edit()`
  - Add `IMGUI_TOOLTIP` to Undo, Redo, Cut, Copy, Paste variants, Delete, Selection tools, BSP duplication, Face splitting, Transform, and Properties.
  - Ensure shortcuts match `ActionRegistry` (`Ctrl+Z`, `Ctrl+Y`, `Ctrl+X`, `Ctrl+C`, `Ctrl+V`, `Del`, `Ctrl+A`, `Esc`, `Ctrl+Alt+A`, `Ctrl+D`, `F`, `Alt+G`, `Ctrl+M`, `Ctrl+G`).
  - _Requirements: REQ-3, REQ-5_

- [x] 3.3 Refactor `drawMenu_View()`
  - Fix `LANG_1095` MenuItem call: `ImGui::MenuItem(get_localized_string(LANG_1095).c_str(), "Ctrl+G", &showGOTOWidget)`.
  - Add `IMGUI_TOOLTIP` to Command Palette, Clipnodes (0..3, Auto), Unhide all, all panel toggles, GOTO, Show Limits, and Skybox.
  - _Requirements: REQ-2.1, REQ-3_

- [x] 3.4 Refactor `drawMenu_Map()`
  - Replace informal nav mesh tooltip with professional technical description.
  - Add `IMGUI_TOOLTIP` to Entity Report, Show Limits, Clean, Optimize, Map Transformations (Mirror, Rotate, Scale), and Recompile Lighting.
  - _Requirements: REQ-3, REQ-7_

- [x] 3.5 Refactor `drawMenu_Tools()`
  - Add `IMGUI_TOOLTIP` to Hull Deletion, Hull Redirection, Cull area hull tools, Fixes (transparent rendering, missing entity classes, unused models, bad leaf count, missing textures, light entities), Additional tools (Delete internal textures, map overlay), and Experimental tools.
  - _Requirements: REQ-3, REQ-7_

- [x] 3.6 Refactor `drawMenu_Create()`
  - Add `IMGUI_TOOLTIP` to Point Entity, Passable Model, Solid Model, Trigger Model, Clip Model, and Random DM spawn points.
  - _Requirements: REQ-3_

- [x] 3.7 Refactor `drawMenu_Windows()`, `drawMenu_Help()`, and `drawMenu_Debug()`
  - Add `IMGUI_TOOLTIP` to Console, Map Tab Switchers, View Help, About, Print Textures, and Create Skybox.
  - _Requirements: REQ-3_

---

## Phase 4: Viewport Context Menu Refactoring (`GuiContextMenu.cpp`)

- [x] 4.1 Refactor Context Menu Labels
  - Replace shortcut-named menu items (`LANG_0441: "Ctrl+V"` -> `"Paste with BSP Model"`, `LANG_0473: "ALT+G"` -> `"Grab / Move"`).
  - Fix `"With intenal textures"` typos.
  - _Requirements: REQ-2.4, REQ-4_

- [x] 4.2 Add Universal Tooltip Coverage to Context Menus
  - Add `IMGUI_TOOLTIP` calls to all Face Selection items (texture copying/pasting, style cloning, lightmap editing, linked selection, face deletion).
  - Add `IMGUI_TOOLTIP` calls to all Object Selection items (origin alignment, clipboard operations, collision hull management, BSP duplication, unhide/hide, properties).
  - _Requirements: REQ-4_

---

## Phase 5: Verification & Quality Assurance

- [x] 5.1 Compilation & Build Verification
  - Run CMake build in Release mode: `cmake --build build --config Release`.
  - Verify zero compilation errors and zero warnings.
  - _Requirements: REQ-6.1_

- [x] 5.2 Menu Inspection & Layout Validation
  - Launch application and verify that:
    1. Top menu bar renders without clipped text or broken alignment.
    2. `Go to...` displays cleanly with `Ctrl+G` properly aligned to the right.
    3. Hovering over every menu and submenu item displays a clear, informative tooltip after the configured delay.
    4. Viewport right-click context menu options display correct titles and tooltips.
  - _Requirements: REQ-6.2_
