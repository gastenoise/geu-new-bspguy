# Requirements Document: Menu Reordering & Refactoring

## Introduction
The purpose of this project is to reorganize and refactor the user interface menu hierarchy of `revamped-newbspguy`. The current menu implementation in `src/editor/Gui.cpp` consists of a monolithic 4,500+ line function (`drawMenuBar`) that mixes file actions, map transformations, collision editing, and debugging tools without standard UX structure.

This document establishes the functional and non-functional requirements for transforming the menu into a standard 8-category top menu bar (**File, Edit, View, Map, Tools, Create, Windows, Help**) and reorganized viewport context menus.

---

## Glossary
- **ImGui**: Immediate Mode Graphical User Interface library used for rendering the application GUI.
- **BSP**: Binary Space Partitioning map format used by GoldSrc / Quake engines.
- **Clipnode**: Collision tree nodes used by GoldSrc for entity and player movement hull collision.
- **Hull**: Specific collision hull index (0=Point/Visual, 1=Humanoid, 2=Large Monster/Vehicle, 3=Head).
- **Cull Box**: Area defined by `cull` entities to perform region-specific editing and deletion.
- **WAD**: Texture archive file used by GoldSrc engine.
- **LIT**: Quake/GoldSrc external lightmap file.

---

## Requirements

### Requirement 1: Top Main Menu Bar Structure
**User Story:** As a map designer, I want a standard top menu bar layout so that I can quickly find features based on standard 3D editor conventions.

#### Acceptance Criteria
1. THE system SHALL render exactly 8 top-level menus in the main menu bar in the following order: `File`, `Edit`, `View`, `Map`, `Tools`, `Create`, `Windows`, `Help`.
2. WHEN developer/debug mode is enabled, THE system SHALL display an additional `(DEBUG)` menu at the far right.
3. THE system SHALL maintain full backward compatibility with all existing menu operations and shortcuts.

---

### Requirement 2: File Menu Organization
**User Story:** As a map designer, I want all file I/O, format conversion, import, and export tools grouped logically in the `File` menu.

#### Acceptance Criteria
1. THE `File` menu SHALL contain: `New Map`, `Open` (submenu), `Open Recent` (submenu), `Save`, `Save As...` (preset-grouped submenu), `Export...` (categorized submenu), `Import...` (categorized submenu), `Quick Test in Sven Co-op`, `Map Merge Wizard...`, `Reload Map`, `Validate Map Structure`, `Preferences / Settings...`, `Close Map`, `Close All`, and `Exit`.
2. THE `Save As...` submenu SHALL group engine target formats into clear presets (`GoldSrc BSP30`, `Blue Shift`, `Quake/HL BSP29`, `Modern BSP2/30ext`).
3. THE `Export...` submenu SHALL group export targets into `3D Geometry & Models`, `Lighting & Visibility Data`, `Textures & WAD`, and `Entity Lump (.ent)`.

---

### Requirement 3: Edit Menu & History
**User Story:** As an editor user, I want clipboard and transformation operations consolidated in the `Edit` menu.

#### Acceptance Criteria
1. THE `Edit` menu SHALL contain: `Undo`, `Redo`, `Cut`, `Copy`, `Paste` (submenu with origin choices), `Duplicate`, `Delete`, `Grab / Move Object`, `Transform Object...`, `Align Origin` (submenu), `Select All`, `Deselect All`, `Select Linked...`, `Unhide All`, and `Entity Properties...`.
2. WHEN no entity or face is selected, THE `Edit` menu SHALL disable selection-dependent operations appropriately.

---

### Requirement 4: View Menu Consolidation
**User Story:** As an editor user, I want viewport display modes, render hulls, and panel toggles centralized under a `View` menu.

#### Acceptance Criteria
1. THE system SHALL provide a top-level `View` menu containing: `Clipnodes Render Hull` (submenu), `Toggle Panels / Widgets` (submenu), `Go to Coordinates...`, and `Show Limits Window`.
2. THE `Toggle Panels / Widgets` submenu SHALL allow toggling `Keyvalue Editor`, `Transform Panel`, `Face Properties Panel`, `Texture Browser Panel`, `Lightmap Editor Panel`, `Console / Log Output`, `Debug Panel`, and `Map Overview`.

---

### Requirement 5: Map Menu Streamlining
**User Story:** As a mapper, I want map-wide inspection, optimization, and transformation tools in the `Map` menu.

#### Acceptance Criteria
1. THE `Map` menu SHALL contain: `Entity Report...`, `Check & Show Limits`, `Clean Map Data`, `Optimize Nodes & Clipnodes`, `Deduplicate Shared Models`, `Map Transformations` (submenu), `Recompile Lighting (RAD)...`, and `Generate Navigation Mesh`.
2. THE `Map Transformations` submenu SHALL group map-wide `Rotate 90° Clockwise`, `Rotate 90° Counter-Clockwise`, `Mirror Map (X/Y)`, and `Scale Entire Map...`.

---

### Requirement 6: Tools Menu Engineering
**User Story:** As an advanced BSP engineer, I want specialized BSP repair, hull editing, and cull-box tools grouped under a dedicated `Tools` menu.

#### Acceptance Criteria
1. THE `Tools` menu SHALL contain: `Hull Management` (submenu), `Cull Area Operations` (submenu), `BSP Geometry Repair & Fixes` (submenu), `Out-Of-Bounds (OOB) Cleanup` (submenu), `Internal Textures Manager` (submenu), and `Experimental / WIP Tools` (submenu).
2. ALL experimental/WIP features (`MDL to BSP Converter`, `UnrealMapDrawTool (.umd)`, `Protect Map`, `Make Map Geometry Overlay`) SHALL be placed inside `Experimental / WIP Tools`.

---

### Requirement 7: Create Menu Prefabs
**User Story:** As a mapper, I want to create entities, brush models, and prefabs from a dedicated `Create` menu.

#### Acceptance Criteria
1. THE `Create` menu SHALL contain: `Point / Brush Entity`, `BSP Brush Models` (submenu: Solid, Passable, Trigger, Clip), and `Prefabs & Generators` (submenu: Random DM Spawns, Default Skybox).

---

### Requirement 8: Viewport Context Menus Ergonomics
**User Story:** As an editor user, I want clean, organized right-click context menus in the 3D viewport.

#### Acceptance Criteria
1. THE `Entity Context Menu` SHALL present entity properties, transform, clipboard, model origin alignment, hull operations, transparency fix, and export options in structured sections with visual separators.
2. THE `Face Context Menu` SHALL present texture/style/lightmap copying & pasting, texture selection, linked face selection, face extraction, and deletion.
3. THE `Workspace Context Menu` SHALL present entity creation and paste options when right-clicking empty void space.
