# Proposed Reordered Menu Structure (`revamped-newbspguy`)

This document presents a complete, redesigned menu structure for `revamped-newbspguy`. The proposed hierarchy strictly follows standard 3D/CAD editor User Experience (UX) conventions (**File → Edit → View → Map → Tools → Create → Windows → Help**), logical workflow grouping, clean separation of experimental features, and context-sensitive ergonomics.

---

## 1. Redesigned Top Main Menu Bar

```
Main Menu Bar
├── File
├── Edit
├── View
├── Map
├── Tools
├── Create
├── Windows
└── Help
```

---

### 1.1. File Menu
*Focus: File lifecycle, workspace state, imports, exports, and application settings.*

* **New Map / Project**
* **Open...** *(Submenu)*
  * **Map File (.bsp)**
  * **Model File (.mdl)**
  * **Sprite (.spr)**
  * **XashNT Model (.csm)**
  * **Add WAD File...**
* **Open Recent** *(Submenu listing recent maps)*
* ─── *(Separator)* ───
* **Save** `[Ctrl+S]`
* **Save As...** *(Opens unified Save dialog with engine format selector presets)*
  * *Preset: GoldSrc / Half-Life (BSP30)*
  * *Preset: Blue Shift*
  * *Preset: Quake / GoldSrc BSP29 (Color Lightmap)*
  * *Preset: Quake / GoldSrc BSP29 (Monochrome Lightmap)*
  * *Preset: Extended / Modern Engines (BSP2 / BSP30ext / 2BSP)*
* **Export...** *(Submenu)*
  * **3D Geometry & Models** *(Submenu)*
    * **Wavefront OBJ (.obj)...**
    * **XashNT Model (.csm)...**
    * **StudioModel Data (.smd)...**
    * **Valve Hammer MAP (.map)...**
    * **Export Selected Entity as BSP Model (.bsp)...**
  * **Lighting & Visibility Data** *(Submenu)*
    * **Lighting LIT File (.lit)**
    * **VIS Portal File (.prt)**
    * **RAD Data Files (.ext / .wa_)**
  * **Textures & WAD** *(Submenu)*
    * **Extract Embedded Textures to WAD**
    * **WAD Textures to PNG List**
    * **Dump Texture Memory Usage**
  * **Entity Lump (.ent)**
* **Import...** *(Submenu)*
  * **BSP Model (Native)**
  * **BSP Model (As func_breakable)**
  * **Convert JMF to BSP**
  * **Entity Data (.ent)**
  * **Lighting LIT File (.lit)**
  * **Textures from WAD / PNG...**
* ─── *(Separator)* ───
* **Quick Test in Sven Co-op** `[F5]` *(Formerly 'Sven Test')*
* **Map Merge Wizard...**
* ─── *(Separator)* ───
* **Reload Map** `[F5 / Ctrl+R]`
* **Validate Map Structure**
* **Preferences / Settings...**
* ─── *(Separator)* ───
* **Close Map** `[Ctrl+W]`
* **Close All**
* **Exit** `[Alt+F4]`

---

### 1.2. Edit Menu
*Focus: History, clipboard, object selection, transformations, and properties.*

* **Undo** `[Ctrl+Z]`
* **Redo** `[Ctrl+Y]`
* ─── *(Separator)* ───
* **Cut** `[Ctrl+X]`
* **Copy** `[Ctrl+C]`
* **Paste** *(Submenu)*
  * **Paste at Cursor / Center** `[Ctrl+V]`
  * **Paste preserving Original Coordinates**
  * **Paste with BSP Model**
  * **Paste at Selected Entity Origin**
* **Duplicate** `[Ctrl+D]` *(Formerly 'Duplicate BSP model')*
* **Delete** `[Del]`
* ─── *(Separator)* ───
* **Grab / Move Object** `[Alt+G]`
* **Transform Object...** `[Ctrl+M]` *(Opens Transform Widget)*
* **Align Origin** *(Submenu)*
  * **Center Origin**
  * **Align Top / Bottom / Left / Right / Front / Back**
* ─── *(Separator)* ───
* **Select All** `[Ctrl+A]`
* **Deselect All** `[Esc]`
* **Select Linked / Connected...** *(Submenu: Depth 1..5)*
* **Unhide All** `[Ctrl+Alt+H]`
* ─── *(Separator)* ───
* **Entity Properties...** `[Alt+Enter]`

---

### 1.3. View Menu *(New Consolidated Top Menu)*
*Focus: Viewport modes, clipnode rendering, overlay toggles, and widget displays.*

* **Clipnodes Render Hull** *(Submenu)*
  * **Auto [-1]**
  * **Point [0]**
  * **Human [1]**
  * **Large [2]**
  * **Head [3]**
* ─── *(Separator)* ───
* **Toggle Panels / Widgets** *(Submenu)*
  * **Keyvalue Editor** `[Alt+Enter]`
  * **Transform Panel** `[Ctrl+M]`
  * **Face Properties Panel**
  * **Texture Browser Panel**
  * **Lightmap Editor Panel**
  * **Console / Log Output** `[~]`
  * **Debug Information Panel**
  * **Map Overview Window**
* ─── *(Separator)* ───
* **Go to Coordinates...** `[Ctrl+G]`
* **Show Limits Window**

---

### 1.4. Map Menu
*Focus: Map-wide structural inspection, cleaning, optimization, and global transformations.*

* **Entity Report...**
* **Check & Show Limits**
* ─── *(Separator)* ───
* **Clean Map Data** *(Removes unused textures, vis, vertices, and models)*
* **Optimize Nodes & Clipnodes**
* **Deduplicate Shared Models**
* ─── *(Separator)* ───
* **Map Transformations** *(Submenu)*
  * **Rotate 90° Clockwise**
  * **Rotate 90° Counter-Clockwise**
  * **Mirror Map (X/Y Swap)**
  * **Scale Entire Map...**
* ─── *(Separator)* ───
* **Recompile Lighting (RAD)...**
* **Generate Navigation Mesh**

---

### 1.5. Tools Menu *(New Dedicated Engineering Menu)*
*Focus: Advanced BSP manipulation, hull editing, surface fixes, cull box operations, and repair routines.*

* **Hull Management** *(Submenu)*
  * **Delete Hull** *(Submenu: Hull 0, 1, 2, 3)*
  * **Redirect Hull** *(Submenu: Hull i -> Hull k)*
  * **Simplify Collision Hulls**
  * **Print Hull Tree / Headnodes to Log**
* **Cull Area Operations** *(Submenu)*
  * **Place Cull Point Entity**
  * **Delete Cull Entities**
  * **Delete Data in Cull Box**
  * **Select Entities in Cull Box**
  * **Select Faces in Cull Box**
  * **Delete Hulls in Cull Area** *(Submenu: Point, Human, Large, Head, Clipnodes, All)*
  * **Create Hulls in Cull Area** *(Submenu: Point, Human, Large, Head, Clipnodes, All)*
  * **Delete Cull Faces** *(Submenu: Sky Leafs, Solid Leafs, Specific Leaf)*
* **BSP Geometry Repair & Fixes** *(Submenu)*
  * **Fix Bad Surface Extents** *(Submenu: Shrink 512, 256, 128, 64, Scale, Subdivide)*
  * **Fix Missing Entity Classes**
  * **Fix Swapped Leaf Mins/Maxs**
  * **Fix Swapped Model Mins/Maxs**
  * **Fix Bad Face Reference in Marksurfaces**
  * **Fix Invalid BSP Face References**
  * **Fix Unused Models (Attach to func_wall)**
  * **Fix Bad Leaf Count**
  * **Fix Texture Overrun Data**
  * **Fix Missing Textures (Replace with White)**
  * **Fix Light Entities Data**
* **Out-Of-Bounds (OOB) Cleanup** *(Submenu)*
* **Internal Textures Manager** *(Submenu: Delete internal, Downscale invalid, etc.)*
* ─── *(Separator)* ───
* **Experimental / WIP Tools** *(Submenu)*
  * **MDL to BSP Converter** *(Submenu: Bruteforce, Compile clipnodes, Meshes to brushes)*
  * **UnrealMapDrawTool (.umd) Generator**
  * **Protect Map (Obfuscate BSP)**
  * **Make Map Geometry Overlay**

---

### 1.6. Create Menu
*Focus: Adding new entities, BSP brush models, and prefabricated structures.*

* **Point / Brush Entity** `[Shift+A]`
* ─── *(Separator)* ───
* **BSP Brush Models** *(Submenu)*
  * **Solid Model**
  * **Passable Model**
  * **Trigger Model**
  * **Clip Collision Model**
* ─── *(Separator)* ───
* **Prefabs & Generators** *(Submenu)*
  * **Random Deathmatch Spawn Points**
  * **Default 6-Side Skybox (Debug)**

---

### 1.7. Windows Menu
*Focus: Multi-tab document navigation and window layouts.*

* **Developer Console** `[~]`
* ─── *(Separator)* ───
* *(Active Map Tabs List: map1.bsp, map2.bsp, ...)*

---

### 1.8. Help Menu
*Focus: Documentation, shortcuts guide, about, and developer options.*

* **Documentation & User Guide** `[F1]`
* **Keyboard Shortcuts Cheat Sheet**
* ─── *(Separator)* ───
* **About revamped-newbspguy**
* ─── *(Separator)* ───
* **Developer & Debug Mode** `[Toggle]` *(Enables (DEBUG) tools)*

---

## 2. Redesigned Viewport Context Menus

### 2.1. Entity / Model Context Menu (Right-Clicking Entity/Model)
```
Entity Context Menu
├── Edit Properties [Alt+Enter]
├── Transform... [Ctrl+M]
├── Grab / Move [Alt+G]
├── ───
├── Cut [Ctrl+X]
├── Copy [Ctrl+C]
├── Paste... [Ctrl+V]
├── Duplicate [Ctrl+D]
├── Delete [Del]
├── Hide [Ctrl+H] / Unhide
├── ───
├── Model Origin Actions
│   ├── Center Origin
│   └── Align Origin (Top/Bottom/Left/Right/Front/Back)
├── Hull Operations
│   ├── Create Hull...
│   ├── Delete Hull...
│   ├── Simplify Hull...
│   ├── Redirect Hull...
│   └── Print Hull Tree
├── Fix Transparent Rendering
└── Export Selected Model as BSP...
```

---

### 2.2. Face Context Menu (Right-Clicking Geometry Face)
```
Face Context Menu
├── Face Properties...
├── ───
├── Copy Texture
├── Paste Texture [Ctrl+V]
├── Copy Face Style
├── Paste Face Style
├── Copy Lightmap
├── Paste Lightmap
├── ───
├── Select Same Texture
├── Select Entire Model
├── Select Linked Faces (Depth 1..5)
├── ───
├── Extract Selected Faces to New Model
└── Delete Selected Faces
```

---

### 2.3. Empty Workspace Context Menu (Right-Clicking Void)
```
Workspace Context Menu
├── Create Entity... [Shift+A]
├── Create Solid BSP Model
├── Create Trigger BSP Model
├── ───
├── Paste [Ctrl+V]
├── Paste at Original Coordinates
└── Paste with BSP Model
```

---

## 3. Key Improvements & UX Rationale

| Category | Original Issues | Proposed Solution | UX Benefit |
| :--- | :--- | :--- | :--- |
| **Top Menu Hierarchy** | Lack of a dedicated `Tools` or `View` top menu; everything placed in `Map` or `Widgets`. | Added standard `View` and `Tools` top menus. | Aligns with 3D editor standards (Blender, Hammer, Radiant). |
| **Save & Export** | 13 unorganized BSP save formats listed flat under `Save as`. | Grouped by target engines (GoldSrc, Blue Shift, Modern BSP2/30ext) and format type. | Eliminates user confusion when exporting to specific engine targets. |
| **Fixes & Repair Utilities** | Scattered across `Map -> Additional tools`, `Map -> Fixes`, and standalone items. | Consolidated into `Tools -> BSP Geometry Repair & Fixes`. | One-stop location for fixing map errors and surface bugs. |
| **Hull Operations** | Mixed between `Map`, context menus, and cull tools. | Grouped under `Tools -> Hull Management`. | Clean separation of collision editing workflows. |
| **WIP & Experimental Features** | Interspersed directly in production menus with inconsistent `[WIP]` tags. | Isolated under `Tools -> Experimental / WIP Tools` or `File -> Export -> ...`. | Prevents accidental invocation of unstable tools during regular editing. |
| **Duplicate Menu Entries** | `Log` listed twice under `Widgets`. | Removed duplicate; assigned global toggle in `View` and `Windows`. | Streamlined interface state management. |
