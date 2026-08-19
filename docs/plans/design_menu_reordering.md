# Design Document: Menu Architecture & Refactoring Strategy

## Overview
This document specifies the technical design for refactoring the ImGui menu rendering system in `revamped-newbspguy`. The refactoring breaks down the monolithic `Gui::drawMenuBar()` method in `src/editor/Gui.cpp` into modular, single-responsibility C++ functions, updates the `Gui` class interface in `src/editor/Gui.h`, and reorganizes localized string lookups.

---

## Architecture & Class Design

### Refactored Class Method Declarations (`src/editor/Gui.h`)

```cpp
class Gui {
public:
    void draw();
    void drawMenuBar();
    void drawBspContexMenu();

private:
    // Main Menu Bar Modular Renderers
    void drawMenu_File();
    void drawMenu_Edit();
    void drawMenu_View();
    void drawMenu_Map();
    void drawMenu_Tools();
    void drawMenu_Create();
    void drawMenu_Windows();
    void drawMenu_Help();
    void drawMenu_Debug();

    // Context Menu Modular Renderers
    void drawContextMenu_Entity();
    void drawContextMenu_Face();
    void drawContextMenu_Empty();
};
```

---

## Component Map & Responsibility Table

| Function | Responsible For | File & Context |
| :--- | :--- | :--- |
| `drawMenuBar()` | Manages `ImGui::BeginMainMenuBar()` lifecycle and delegates to 8 menu renderers. | `Gui.cpp` (Main Bar) |
| `drawMenu_File()` | Handles File I/O, Save As presets, Export submenus, Import submenus, Sven Test, Merge, Settings, Exit. | `Gui.cpp` |
| `drawMenu_Edit()` | Handles Undo/Redo, Clipboard (Cut/Copy/Paste), Grab/Transform, Alignment, Select Linked, Properties. | `Gui.cpp` |
| `drawMenu_View()` | Handles Clipnodes render hull selector, Widget panel toggles, GoTo, Limits window toggle. | `Gui.cpp` |
| `drawMenu_Map()` | Handles Entity Report, Limits check, Clean, Optimize, Map-wide transforms, RAD recompile, Nav mesh. | `Gui.cpp` |
| `drawMenu_Tools()` | Handles Hull management, Cull Box area ops, BSP repair/fixes, OOB cleanup, WIP tools. | `Gui.cpp` |
| `drawMenu_Create()` | Handles Entity spawning, BSP brush models (Solid/Passable/Trigger/Clip), Prefabs/Skybox. | `Gui.cpp` |
| `drawMenu_Windows()` | Handles Console toggle and active map tab navigation. | `Gui.cpp` |
| `drawMenu_Help()` | Handles Documentation link, Shortcuts cheat sheet, About dialog, Debug mode toggle. | `Gui.cpp` |
| `drawBspContexMenu()` | Delegates viewport right-click popups (`ent_context`, `face_context`, `empty_context`). | `Gui.cpp` (Context) |

---

## Data Flow & State Dependencies

```
[ImGui Frame] 
    │
    ├── Gui::drawMenuBar() ──> [ImGui::BeginMainMenuBar()]
    │     ├── drawMenu_File()    ──> checks (map, isLoading, g_settings)
    │     ├── drawMenu_Edit()    ──> checks (canUndo, canRedo, pickInfo)
    │     ├── drawMenu_View()    ──> checks (clipnodeRenderHull, widget flags)
    │     ├── drawMenu_Map()     ──> checks (map, is_mdl_model)
    │     ├── drawMenu_Tools()   ──> checks (cullbox, map, rend)
    │     ├── drawMenu_Create()  ──> checks (map)
    │     ├── drawMenu_Windows() ──> checks (mapRenderers)
    │     └── drawMenu_Help()    ──> checks (showAboutWidget, etc.)
    │
    └── Gui::drawBspContexMenu() ──> [ImGui::BeginPopup()]
          ├── drawContextMenu_Entity()
          ├── drawContextMenu_Face()
          └── drawContextMenu_Empty()
```

---

## Localized Strings & Configuration (`language.ini`)

All menu items will use localized strings fetched via `get_localized_string(LANG_XXXX)` or new localization keys where required:
- `LANG_MENU_VIEW` = "View"
- `LANG_MENU_TOOLS` = "Tools"
- `LANG_MENU_PREFABS` = "Prefabs & Generators"
- `LANG_MENU_EXPERIMENTAL` = "Experimental / WIP Tools"
