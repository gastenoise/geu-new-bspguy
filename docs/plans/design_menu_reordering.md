# Design Document: Modular GUI Architecture & Menu Refactoring

## Overview
This document specifies the technical design for decomposing the monolithic `src/editor/Gui.cpp` (15,500+ lines) into a modular, clean C++ architecture under `src/editor/gui/`. It details the target file layout, class method declarations, CMake integration, and localization mapping.

---

## File System & Modular Architecture

### New Directory & File Decomposition Plan

```
src/editor/
├── Gui.h                     # Main Gui class declaration
├── Gui.cpp                   # Core Gui lifecycle, initialization & dispatching
└── gui/                      # NEW MODULAR GUI DIRECTORY
    ├── GuiMenuBar.cpp        # Main Menu Bar rendering (File, Edit, View, Map, Tools, Create, Windows, Help)
    ├── GuiContextMenu.cpp    # Viewport Context Popups (Entity, Face, Empty Workspace)
    ├── GuiWidgets.cpp        # Floating Widgets (Log, Debug, Transform, Keyvalue Editor, etc.)
    └── GuiDialogs.cpp        # Modal Dialogs (Settings, About, Help, Merge Wizard)
```

---

## Refactored Class Interface (`src/editor/Gui.h`)

```cpp
#pragma once
#include <string>
#include <vector>
// ImGui & Application dependencies

class Gui {
public:
    Gui();
    ~Gui();

    void draw();
    void drawMenuBar();
    void drawBspContexMenu();

private:
    // Modular Main Menu Bar Renderers (defined in src/editor/gui/GuiMenuBar.cpp)
    void drawMenu_File();
    void drawMenu_Edit();
    void drawMenu_View();
    void drawMenu_Map();
    void drawMenu_Tools();
    void drawMenu_Create();
    void drawMenu_Windows();
    void drawMenu_Help();
    void drawMenu_Debug();

    // Modular Context Menu Renderers (defined in src/editor/gui/GuiContextMenu.cpp)
    void drawContextMenu_Entity();
    void drawContextMenu_Face();
    void drawContextMenu_Empty();

    // Modular Widget Renderers (defined in src/editor/gui/GuiWidgets.cpp & GuiDialogs.cpp)
    void drawLog();
    void drawHelp();
    void drawAbout();
    void drawSettings();
    void drawKeyvalueEditor();
    void drawTextureBrowser();
    void drawLightMapTool();
    void drawTransformWidget();
    void drawGOTOWidget();
    void drawOverviewWidget();
    void drawMergeWindow();
    void drawImportMapWidget();
};
```

---

## Build System Integration (`CMakeLists.txt`)

Register the new modular files in `CMakeLists.txt`:

```cmake
# 3D editor GUI modules
src/editor/Gui.h                   src/editor/Gui.cpp
src/editor/gui/GuiMenuBar.cpp
src/editor/gui/GuiContextMenu.cpp
src/editor/gui/GuiWidgets.cpp
src/editor/gui/GuiDialogs.cpp
```

---

## Component Responsibilities & Line Budget

| Target File | Responsibilities | Target Line Count |
| :--- | :--- | :--- |
| `src/editor/Gui.cpp` | `Gui::draw()` main loop, window docking, theme initialization. | ~1,500 lines |
| `src/editor/gui/GuiMenuBar.cpp` | Top Menu Bar rendering (`File`, `Edit`, `View`, `Map`, `Tools`, `Create`, `Windows`, `Help`, `Debug`). | ~2,500 lines |
| `src/editor/gui/GuiContextMenu.cpp` | Viewport right-click context popups (`ent_context`, `face_context`, `empty_context`). | ~1,200 lines |
| `src/editor/gui/GuiWidgets.cpp` | Floating tool windows (Keyvalue editor, Texture browser, Lightmap editor, Transform). | ~6,000 lines |
| `src/editor/gui/GuiDialogs.cpp` | Modal dialogs (Settings, About, Help, Map Import, Map Merge Wizard). | ~4,000 lines |
