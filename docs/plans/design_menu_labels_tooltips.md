# Design Document: Menu Labels Sanitization & Universal Tooltip Coverage

## Overview

This document outlines the technical design for eliminating label glitches, fixing translation bugs, standardizing shortcut rendering, and implementing 100% hover tooltip coverage across the GUI system of `revamped-newbspguy`. The design integrates seamlessly with the existing immediate-mode UI framework (`ImGui`), the localization catalog system (`lang.h`/`language.ini`), and the centralized command registry (`ActionRegistry`).

---

## System Architecture

### Component Map

| Component ID | Name | Type | Responsibility | Interfaces With |
| :--- | :--- | :--- | :--- | :--- |
| **COMP-1** | `GuiMenuBar` | C++ GUI Module | Renders top main menu bar (8 standard categories + Debug) | `Gui`, `Bsp`, `Renderer`, `Settings`, `lang` |
| **COMP-2** | `GuiContextMenu` | C++ GUI Module | Renders 3D viewport right-click context menus (Face, Object, Empty) | `Gui`, `Bsp`, `Renderer`, `lang` |
| **COMP-3** | `ActionRegistry` | C++ Subsystem | Centralized action definitions, descriptions, and keybindings | `GuiCommandPalette`, `GuiMenuBar` |
| **COMP-4** | `LocalizationManager` | Core Utility (`lang.cpp`/`ini.h`) | Runtime string translation and lookup by `LANG_XXXX` enum | `language*.ini` files |
| **COMP-5** | `ImGui Tooltip Engine` | Immediate Mode UI | Renders delayed hover popups when `HoveredIdTimer > g_tooltip_delay` | `ImGuiContext`, `GImGui` |

### High-Level Architecture Diagram

```
+-------------------------------------------------------------------------+
|                               Renderer / Gui                            |
|                                                                         |
|  +---------------------------+        +------------------------------+  |
|  |     GuiMenuBar.cpp        |        |      GuiContextMenu.cpp      |  |
|  | (File, Edit, View, Map,   |        |  (Face, Object, Selection    |  |
|  |  Tools, Create, Win, Help)|        |        Context Menus)        |  |
|  +-------------+-------------+        +--------------+---------------+  |
|                |                                     |                  |
|                +------------------+------------------+                  |
|                                   |                                     |
|                   +---------------+---------------+                     |
|                   |                               |                     |
|                   v                               v                     |
|       +-----------------------+       +-----------------------+         |
|       | LocalizationManager   |       |  IMGUI_TOOLTIP Engine |         |
|       | (get_localized_string)|       | (HoveredIdTimer delay)|         |
|       +-----------+-----------+       +-----------+-----------+         |
|                   |                               |                     |
|                   v                               v                     |
|         resources/languages/              ImGui Render Output           |
|        (EN, RU, ZH catalogs)              (Tooltip Popups)              |
+-------------------------------------------------------------------------+
```

---

## Data Flow Specifications

### Tooltip Display Lifecycle

```
1. User moves cursor over an ImGui::MenuItem or ImGui::BeginMenu item.
2. ImGui tracks item state and increments g.HoveredIdTimer for the active item ID.
3. Code invokes IMGUI_TOOLTIP(g, tooltip_string).
4. IF ImGui::IsItemHovered() AND g.HoveredIdTimer > g_tooltip_delay:
   a. ImGui::BeginTooltip() opens a transient overlay window.
   b. ImGui::TextUnformatted(tooltip_string.c_str()) outputs wrapped explanation.
   c. ImGui::EndTooltip() finalizes and positions the popup near the cursor.
```

### Decoupled Shortcut Data Flow

```
Legacy Glitch Flow:
[language.ini] LANG_1095 = "Go to...\tCtrl+G"
                      |
                      v
ImGui::MenuItem(get_localized_string(LANG_1095).c_str(), NULL, &showGOTOWidget)
                      |
                      +---> Rendered Label: "Go to... /tCtrl+G" (Broken formatting / missing shortcut alignment)

Clean Modernized Flow:
[language.ini] LANG_1095 = "Go to..."
                      |
                      v
ImGui::MenuItem(get_localized_string(LANG_1095).c_str(), "Ctrl+G", &showGOTOWidget)
                      |
                      +---> Rendered Label:    "Go to..." (Left column)
                      +---> Rendered Shortcut: "Ctrl+G"   (Right column aligned by ImGui)
```

---

## Technical Components & Interfaces

### 1. Unified Tooltip Rendering Helper

In `src/editor/gui/GuiMenuBar.cpp` and `src/editor/gui/GuiContextMenu.cpp`, ensure the tooltip helper is cleanly defined:

```cpp
static inline void IMGUI_TOOLTIP(ImGuiContext& g, const std::string& text)
{
	if (ImGui::IsItemHovered() && g.HoveredIdTimer > g_tooltip_delay)
	{
		ImGui::BeginTooltip();
		ImGui::TextUnformatted(text.c_str());
		ImGui::EndTooltip();
	}
}
```

### 2. Localization Catalog Updates (`resources/languages/`)

1. **`LANG_1095`**:
   - `language.ini` (EN): `LANG_1095 = Go to...`
   - `language_ru.ini` (RU): `LANG_1095 = Перейти к...`
   - `language_zh.ini` (ZH): `LANG_1095 = 转到...`

2. **Typo Fixes (`"intenal"` -> `"internal"`)**:
   - `LANG_0469`, `LANG_1071`, `LANG_1155`, `LANG_1174`: `"With internal textures[HL1]"`
   - `LANG_0470`, `LANG_1072`, `LANG_1156`, `LANG_1175`: `"With internal textures[QUAKE/HL1+XASH]"`

3. **Context Menu Action Labels**:
   - `LANG_0441`: Label for Paste with BSP model decoupled from raw `"Ctrl+V"`.
   - `LANG_0473`: Label for Grab/Ungrab decoupled from raw `"ALT+G"`.

---

## Comprehensive Tooltip Specifications by Category

### File Menu
- **Save**: Saves all pending entity, texture, and geometry changes directly into the current `.bsp` file.
- **Save As Presets**:
  - `Half Life`: Standard GoldSrc BSP v30 format with embedded 256-color palette.
  - `Half Life [NO PALETTE]`: Standard GoldSrc BSP v30 format without 256-color palette data.
  - `Blue Shift`: Gearbox Blue Shift BSP v30 format with modified header structure.
  - `BSP29 [COLOR LIGHT]`: Quake/Half-Life BSP v29 format with 24-bit RGB colored lightmaps.
  - `BSP29 [MONO LIGHT]`: Quake/Half-Life BSP v29 format with monochrome (greyscale) lightmaps.
  - `BSP29 [BROKEN CLIPNODES]`: Aguirre QBSP format with 16-bit unsigned clipnodes.
  - `HL BSP2 [32 bit]`: Extended 32-bit BSP format supporting massive coordinate spaces.
  - `XASH BSP30ex [32 bit]`: Xash3D Engine extended BSP30 with 32-bit indices and extra lumps.
- **Export Formats**:
  - `SMD`: Exports mesh triangles and bones into Valve StudioModel Data format.
  - `OBJ / CSM`: Exports geometry into Wavefront `.obj` or XashNT `.csm` format.
  - `MAP`: Decompiles BSP geometry into editable Hammer MAP brush solids.
  - `VIS .prt`: Exports portal boundary data for VIS visibility compilation.
  - `RAD .ext/.wa_`: Exports face lighting boundary data for radiosity compilation.
  - `Lighting .lit`: Exports colored lightmap lump data as an external Quake `.lit` file.
  - `BSP Model`: Exports the selected submodel as a standalone `.bsp` file.
  - `Selected Faces as BSP`: Extracts selected face polygons into a new standalone BSP model.
  - `Map Overview Screenshot`: Renders a top-down orthogonal overview image for radar overlays.
  - `WAD to PNG`: Dumps embedded and loaded textures into individual PNG files.
  - `Dump Textures`: Logs all texture names, dimensions, and memory usage to the console.
- **Import Formats**:
  - `BSP Model (native)`: Inserts an external `.bsp` model into the active map with full collision.
  - `BSP Model (func_breakable)`: Inserts an external model wrapped as a breakable entity.
  - `JMF to BSP`: Directly converts a J.A.C.K. map format into compiled BSP structures.
  - `Entity file`: Imports entity definitions and merges/replaces the `.ent` lump.
  - `Textures from WAD`: Embeds external WAD textures directly into the BSP file.
- **Workflow Tools**:
  - `Sven Test`: Fast-exports the BSP and ENT files directly into the Sven Co-op addon folder.
  - `Map Merge Wizard`: Merges multiple BSP maps into a single unified level.
  - `Reload`: Reloads the active map from disk, discarding unsaved viewport changes.
  - `Validate`: Performs structural integrity verification across all BSP lumps.
  - `Settings`: Configures editor preferences, rendering flags, and keybindings.
  - `Close / Close All`: Closes the current or all open map tabs.
  - `Exit`: Closes the editor.

### Edit Menu
- **Undo / Redo**: Reverts or re-applies the previous map editing operation.
- **Cut / Copy / Paste Variants**: Clipboard management for entities, textures, and brush models.
- **Select All / Deselect All**: Selects all non-world entities or clears selection.
- **Faces with Same Texture**: Selects all map faces sharing the picked face's texture.
- **Duplicate BSP Model**: Duplicates the underlying BSP model structures for independent editing.
- **Unlock BSP Model**: Unlinks shared brush models to allow modification without altering other instances.
- **Merge Selected Models**: Merges multiple selected brush models into a single combined entity model.
- **Split Face**: Subdivides a picked face along selected edge vertex points.
- **Grab / Move**: Interactively drags selected entities in 3D space with the mouse cursor.
- **Transform Tool**: Opens numeric translation, rotation, and scale manipulator.
- **Properties (SmartEdit)**: Opens the entity keyvalue, flags, and attribute editor dialog.

### View Menu
- **Command Palette**: Opens quick fuzzy-search launcher for all commands (`Ctrl+K`).
- **Show Clipnodes (0..3, Auto)**: Toggles collision hull wireframe rendering for point, human, large, or head hulls.
- **Unhide All**: Restores visibility to all temporarily hidden entities and geometry.
- **Toggle Panels**: Individual toggles for Keyvalue Editor, Transform, Face Properties, Texture Browser, Lightmap Editor, Log Console, Debug, and Map Overview.
- **Go to Coordinates**: Teleports the 3D camera to exact coordinates or entity index.
- **Show Limits**: Toggles engine lump and memory usage gauges.
- **Skybox**: Toggles 3D environment skybox cubemap rendering in the viewport.

### Map Menu
- **Entity Report**: Searchable entity table listing classnames, targets, and models.
- **Clean**: Strips unused clipnodes, orphaned models, and empty leaves.
- **Optimize**: Compacts vertex tables, merges collinear edges, and sorts marksurfaces.
- **Map Transformations**: Mirrors, rotates (90° CW/CCW), or scales world geometry and entities.
- **Recompile Lighting**: Triggers external radiosity compiler on the active map.
- **Generate Nav Mesh**: Generates bot navigation mesh graph from walkable BSP leaves.

### Tools Menu
- **Delete / Redirect Hull**: Strips or redirects physics collision hulls to reduce clipnode usage.
- **Cull Actions**: Boxed area operations for deleting or selecting entities, faces, and collision hulls.
- **Fixes**: Comprehensive repairs for transparency artifacts, missing entity classes, bad surface extents, inverted bounding boxes, invalid face references, orphaned models, bad leaf counts, texture overrun, and missing textures.
- **Additional Tools**: Strips out-of-bounds (OOB) data, strips internal textures to external WAD, deduplicates identical models, and scales/subdivides textures to fix bad extents.
- **Experimental Tools**: Converts MDL studio models to BSP brush geometry and applies map protection.

### Create Menu
- **Entity**: Places point entities (`info_player_start`, lights, ambient sounds) at camera target.
- **BSP Passable / Solid / Trigger / Clip Models**: Creates illusionary, solid, trigger, or player-clip brush models.
- **Random DM Spawn Points**: Generates distributed deathmatch spawn locations on walkable surfaces.

### Windows, Help & Debug Menus
- **Console**: Toggles embedded developer output console.
- **Map Tab Switcher**: Switches active view to opened map files.
- **View Help / About**: Displays shortcut reference and developer credits.
- **Print Textures**: Dumps loaded texture memory layout to debug log.
- **Create Skybox**: Generates 6-sided bounding skybox solid around the entire map perimeter.

---

## Testing & Quality Assurance Strategy

1. **Syntax & Compilation Validation**: Full clean build with MSVC C++20.
2. **Visual Inspection**:
   - Inspect every menu category in the menu bar.
   - Verify hover tooltips appear after `g_tooltip_delay` seconds.
   - Verify `Ctrl+G` shortcut is properly aligned in the right column and label is cleanly formatted.
   - Verify all context menu items display proper labels and tooltips.
