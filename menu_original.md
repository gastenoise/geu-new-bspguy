# Complete Original Menu Structure (`geu-new-bspguy`)

This document provides a comprehensive, end-to-end breakdown of all top-level main menu bar items, submenus, options, and viewport context menus in the application as extracted directly from the codebase (`src/editor/Gui.cpp`) and localized string definitions (`resources/languages/language.ini`).

---

## 1. Top Main Menu Bar (`ImGui::BeginMainMenuBar`, `Gui.cpp:L2286-L6903`)

### 1.1. File Menu (`LANG_0478`, `Gui.cpp:L2364`)
* **Save** (`LANG_0479`, L2366) `[Shortcut: None]` *(Disabled if map loading or MDL model)*
* **Save as** (`LANG_0480`, L2373) *(Submenu)*
  * **Half Life** (`LANG_0481`, L2392) — Standard BSP30 format with palette.
  * **Half Life [NO PALETTE]** (`LANG_0481`, L2427) — Standard BSP30 format without palette.
  * **Blue Shift** (`LANG_0485`, L2480) — Gearbox Blue Shift BSP30 compatibility format.
  * **Half-Life BSP29 [COLOR LIGHT]** (`LANG_0489`, L2533) — Quake/HL BSP29 with colored lightmaps.
  * **Half-Life BSP29 [MONO LIGHT]** (`LANG_0493`, L2586) — Quake/HL BSP29 with monochrome lightmaps.
  * **HL BSP29 [BROKEN CLIPNODES] [COLOR LIGHT]** (`LANG_0497`, L2641) — BSP29 with 16-bit unsigned clipnodes.
  * **HL BSP29 [BROKEN CLIPNODES] [MONO LIGHT]** (`LANG_0501`, L2694) — BSP29 with broken clipnodes & mono light.
  * **HL BSP2 [32 bit] [COLOR LIGHT]** (`LANG_0505`, L2749) — BSP2 extended 32-bit format.
  * **HL BSP2 [32 bit] [MONO LIGHT]** (`LANG_0506`, L2804) — BSP2 extended 32-bit with mono light.
  * **HL 2BSP [OLD] [32 bit] [COLOR LIGHT]** (`LANG_0507`, L2858) — Legacy 2BSP format with color light.
  * **HL 2BSP [OLD] [32 bit] [MONO LIGHT]** (`LANG_0511`, L2912) — Legacy 2BSP format with mono light.
  * **XASH BSP30ex [32 bit] [COLOR LIGHT]** (`LANG_0515`, L2966) — Xash3D extended BSP30 with color light.
  * **XASH BSP30ex [32 bit] [MONO LIGHT]** (`LANG_0519`, L3020) — Xash3D extended BSP30 with mono light.
* **Open** (`LANG_0523`, L3089) *(Submenu)*
  * **Map (.bsp)** (`LANG_0524`, L3091)
  * **Model (.mdl)** (`LANG_0526`, L3105)
  * **Sprite (.spr)** (`OPEN_SPR_VIEW`, L3118)
  * **XashNT (.csm)** (`OPEN_XASHNT_CSM_VIEW`, L3131)
  * **Wad** (`LANG_0528`, L3144) — Adds WAD path to current map's `worldspawn`.
* **Close** (`LANG_0530`, L3159) — Closes currently active map tab.
* **Close All** (`LANG_0531`, L3191) — Closes all open map tabs.
* **Export** (`LANG_0532`, L3216) *(Submenu)*
  * **Entity file** (`LANG_0533`, L3218) — Exports `.ent` text lump.
  * **All embedded textures to wad** (`LANG_0534`, L3238) — Extracts map textures to `.wad`.
  * **StudioModel Data (.smd) [WIP]** (L3261) *(Submenu)*
    * **Split to goldsrc** (L3263) `[Checkbox]`
    * **Only root bone** (L3267) `[Checkbox]`
    * **Do Export** (L3272)
  * **Wavefront(.obj)/XashNT(.csm) [WIP]** (L3286) *(Submenu)*
    * **Select scale** (L3288) *(Submenu: Scale 1x, 0.5x, 0.25x, 0.125x, 0.0625x, 0.03125x, -1x, etc.)*
    * **Create face groups [OBJ]** (L3316) `[Checkbox]`
    * **Create face objects [OBJ]** (L3322) `[Checkbox]`
    * **Export .obj** (L3328) *(Submenu: Export only bsp, Export with models)*
    * **Export .csm** (L3343) *(Submenu: Export only bsp, Export with models)*
  * **ValveHammerEditor (.map) [WIP]** (L3371) *(Submenu)*
    * **Merge faces** (L3373) `[Checkbox]`
    * **One back vert** (L3375) `[Checkbox]`
    * **Create box** (L3377) `[Checkbox]`
    * **Full .map** (L3381)
    * **Selected faces** (L3387)
  * **VIS .prt file** (`LANG_0537`, L3403) — Exports portal file for REVIS.
  * **RAD.exe .ext & .wa_ files** (`LANG_0539`, L3417)
  * **Lighting .lit file** (`LANG_0540`, L3433)
  * **Export BSP model** (`LANG_1076`, L3447) *(Submenu)*
    * **Export Model [X].bsp** (L3457) *(Submenu)*
      * **With origin** (`LANG_1077`, L3459) -> *(With WAD, With internal textures [HL1], With internal textures [QUAKE/HL1+XASH])*
      * **Without origin** (`LANG_1078`, L3475) -> *(With WAD, With internal textures [HL1], With internal textures [QUAKE/HL1+XASH])*
  * **WAD [to .png list]** (`LANG_0542`, L3498) *(Submenu listing loaded WADs for PNG export)*
  * **UnrealMapDrawTool (.umd) [WIP]** (L3519) *(Submenu)*
    * **Options###1** (L3527) *(Submenu: [Scan] Cell size (16, 32, 64, 128, 256, 512), Support textures, Fill near faces, Scan faces, NO OPTIMIZE)*
    * **Do Export [MAP]###2** (L3615)
    * **Do Export [HEAD_HULL]###3** (L3621)
  * **Dump Textures** (L4270)
* **Import** (`LANG_0543`, L4303) *(Submenu)*
  * **BSP model (native)** (`LANG_0544`, L4305)
  * **BSP model (cached as func_breakable)** (`LANG_0545`, L4311)
  * **Lighting .lit file** (`LANG_0546`, L4317)
  * **Create .BSP from .JMF** (L4330)
  * **Entity file** (`LANG_0533`, L4339)
  * **Import all textures from wad** (`LANG_0547`, L4383)
  * **Embedded##import** (L4398) *(Submenu)*
    * **Enable Dithering ##1** (`LANG_0549`, L4400) `[Toggle]`
    * **From .png files** (L4403)
  * **WAD** (`LANG_0548`, L4420) *(Submenu)*
    * **Enable Dithering ##1** (`LANG_0549`, L4422) `[Toggle]`
    * *(List of WAD files to import)*
* **Sven Test** (`LANG_0550`, L4449) — Fast export BSP/ENT to Sven Co-op addon folder.
* **Merge** (L4478) — Opens map merge wizard dialog.
* **Recent Files** (L4493) *(Submenu listing recently opened map paths)*
* **Reload** (`LANG_0552`, L4510) — Reloads active map file.
* **Validate** (`LANG_0553`, L4516) — Validates map structure integrity.
* **Settings** (`LANG_0554`, L4528) — Opens global editor settings modal.
* **Exit** (`LANG_0555`, L4537) — Terminates the application.

---

### 1.2. Edit Menu (`LANG_0556`, `Gui.cpp:L4544`)
* **Undo [Action]** (`LANG_0557`, L4567) `[Shortcut: Ctrl+Z]`
* **Redo [Action]** (`LANG_0558`, L4571) `[Shortcut: Ctrl+Y]`
* **Cut** (`LANG_1081`, L4578) `[Shortcut: Ctrl+X]`
* **Copy** (`LANG_1083`, L4582) `[Shortcut: Ctrl+C]`
* **Paste** (`LANG_0449`, L4589) *(Submenu)*
  * **Paste** (L4591) `[Shortcut: Ctrl+V]`
  * **Paste with origin** (`LANG_0450`, L4595)
  * **Paste with bspmodel** (L4599) `[Shortcut: Ctrl+V]`
* **Delete** (`LANG_1085`, L4606) `[Shortcut: Del]`
* **Unhide all** (`LANG_0559`, L4610) `[Shortcut: Ctrl+Alt+H]`
* **Duplicate BSP model** (`LANG_DUPLICATE_BSP`, L4638)
* **Unlock BSP model** (`LANG_DUPLICATE_BSP_STRUCT`, L4664)
* **Grab / Ungrab** (`LANG_1087` / `LANG_1088`, L4705) `[Shortcut: Alt+G]`
* **Transform** (`LANG_1089`, L4714) `[Shortcut: Ctrl+M]` — Opens transform widget.
* **Properties** (`LANG_1091`, L4721) `[Shortcut: Alt+Enter]` — Opens entity keyvalue editor.

---

### 1.3. Map Menu (`LANG_0561`, `Gui.cpp:L4729`)
* **Entity Report** (`LANG_0562`, L4731) — Opens entity list report window.
* **Show Limits** (`LANG_0563`, L4736) — Displays current BSP lump usage vs engine limits.
* **Clean** (`LANG_0564`, L4744) — Cleans unused BSP structures (vis, textures, etc.).
* **Optimize** (`LANG_0565`, L4752) — Optimizes clipnodes and nodes.
* **Show clipnodes** (`LANG_0566`, L4775) *(Submenu)*
  * **[-1] - Auto** (`LANG_0567`, L4778)
  * **[0] - Point** (`LANG_0568`, L4782)
  * **[1] - Human** (`LANG_0569`, L4786)
  * **[2] - Large** (`LANG_0570`, L4790)
  * **[3] - Head** (`LANG_0571`, L4794)
* **MDL to BSP (WIP)** (L4809) *(Submenu)*
  * **Bruteforce clipnodes** (L4812) `[Checkbox]`
  * **Compile clipnodes** (L4818) `[Checkbox]`
  * **Meshes to brushes** (L4824) `[Checkbox]`
  * **Convert selected to BSP** (L4831)
* **Recompile lighting** (L4862) — Runs external RAD tool.
* **PROTECT MAP! (WIP)** (L4949) — Obfuscates map data.
* **Additional tools** (L4995) *(Submenu)*
  * **Delete OOB Data** (L4997) *(Submenu listing Out-Of-Bounds cleanup modes)*
  * **Delete internal textures** (L5044)
  * **Deduplicate Models** (L5052)
  * **Downscale Invalid Textures (WIP)** (L5060)
  * **Fix Bad Surface Extents** (L5065) *(Submenu: Shrink Textures 512, 256, 128, 64, Scale, Subdivide)*
  * **Make map overlay** (L5127)
* **Cull actions** (`LANG_1185`, L5242) *(Submenu)*
  * **Cull Entity** (`LANG_1186`, L5253) — Drops a `cull` entity at camera focus.
  * **Delete cull ents** (`LANG_1203`, L5266)
  * **Delete Boxed Data** (`LANG_1188`, L5282)
  * **Select Boxed Entities** (`LANG_1190`, L5294)
  * **Select Boxed Faces** (`LANG_1194`, L5304)
  * **Delete Hulls in Cull Area** (`LANG_1192`, L5316) *(Submenu: Point, Human, Large, Head, Clipnodes, All Hulls)*
  * **Create Hulls in Cull Area** (`LANG_1193`, L5352) *(Submenu: Point, Human, Large, Head, Clipnodes, All Hulls)*
  * **Delete cull faces** (`LANG_1196`, L5390) *(Submenu: Delete from [SKY LEAFS], Delete from [SOLID LEAFS], Delete from [leaf])*
* **Generate nav mesh** (L5435)
* **MAP TRANSFORMATION [WIP]** (L5442) *(Submenu)*
  * **Mirror map x/y** (L5444)
  * **Rotate Counter Clockwise 90** (L5529)
  * **Rotate Clockwise 90** (L5656)
  * **Scale map** (L5783) *(Submenu: Scale selected, Scale presets)*
* **Delete Hull** (`LANG_1093`, L5927) *(Submenu: Hull 0, Hull 1, Hull 2, Hull 3)*
* **Redirect Hull** (`LANG_1094`, L5946) *(Submenu: Redirect Hull i to Hull k)*
* **Fixes** (`LANG_0572`, L5974) *(Submenu)*
  * **Missing entities classes** (L5976)
  * **Bad surface extents** (`LANG_0573`, L5987)
  * **Swapped leaf mins/maxs** (`LANG_0575`, L6011)
  * **Swapped models mins/maxs** (`LANG_0577`, L6031)
  * **Bad face reference in marksurf** (`LANG_0579`, L6053)
  * **Invalid BSP face references** (`LANG_1183`, L6070)
  * **Unused models** (`LANG_0581`, L6082)
  * **Fix bad leaf count** (L6125)
  * **Texture overrun data** (`LANG_0583`, L6154)
  * **Missing textures** (`LANG_0584`, L6201)
  * **Fix light entities[+TEXTURE]** (L6259)
  * **Fix light entities** (L6273)

---

### 1.4. Create Menu (`LANG_0587`, `Gui.cpp:L6292`)
* **Entity** (`LANG_0588`, L6294) — Spawns new point/brush entity.
* **BSP Passable Model** (`LANG_0589`, L6307) — Creates non-solid brush model.
* **BSP Solid Model** (`LANG_0591`, L6344) — Creates solid brush model.
* **BSP Trigger Model** (`LANG_0590`, L6380) — Creates trigger volume model.
* **BSP Clip model** (L6417) — Creates clipnode-only collision model.
* **Other** (L6455) *(Submenu)*
  * **Random DM spawn points** (L6457)

---

### 1.5. Widgets Menu (`LANG_0592`, `Gui.cpp:L6476`)
* **Log** (`LANG_0594`, L6480) `[Toggle]` — Output console log window.
* **Debug** (`LANG_0595`, L6487) `[Toggle]` — Debug information panel.
* **Keyvalue Editor** (`LANG_0596`, L6491) `[Shortcut: Alt+Enter]` `[Toggle]`
* **Transform** (`LANG_1160`, L6495) `[Shortcut: Ctrl+M]` `[Toggle]`
* **Go to** (`LANG_1095`, L6499) `[Shortcut: Ctrl+G]` `[Toggle]` — Camera coordinate teleport window.
* **Face Properties** (`LANG_0597`, L6504) `[Toggle]` — Face & texture editing panel.
* **Texture Browser** (`LANG_0598`, L6508) `[Toggle]`
* **LightMap Editor** (`LANG_0599`, L6512) `[Toggle]`
* **Map merge** (`LANG_0600`, L6519) `[Toggle]` — Map merge configuration window.
* **Map Overview** (L6523) `[Toggle]` — 2D overview generator panel.
* **Log** (`LANG_1096`, L6527) `[Toggle]` *(Duplicate entry in original menu code)*

---

### 1.6. Windows Menu (`LANG_0601`, `Gui.cpp:L6537`)
* **Console** (L6540) `[Toggle]` — Toggle developer console window.
* *(Dynamic list of loaded map tabs)* (L6550) — Switch focus to specific open map.

---

### 1.7. Help Menu (`LANG_0602`, `Gui.cpp:L6566`)
* **View help** (`LANG_0603`, L6568) — Displays user documentation / help dialog.
* **About** (`LANG_0604`, L6572) — Displays application version and credits.

---

### 1.8. (DEBUG) Menu (`LANG_0605`, `Gui.cpp:L6581`)
* **Print textures** (L6583) — Dumps texture memory stats to log.
* **CREATE SKYBOX** (L6617) — Generates default skybox geometry.

---

## 2. Viewport Context Menus (`ImGui::BeginPopup`)

### 2.1. Entity / Model Context Menu (`ent_context`, `Gui.cpp:L1202 & L1720`)
* **Center** (`LANG_0430`, L1208 & L1301) — Recenters model origin.
* **Align** (`LANG_0431`, L1216 & L1309) *(Submenu)*
  * **Top** (`LANG_0432`)
  * **Bottom** (`LANG_0433`)
  * **Left** (`LANG_0434`)
  * **Right** (`LANG_0435`)
  * **Back** (`LANG_0436`)
  * **Front** (`LANG_0437`)
* **Fix transparent rendering** (L1273 & L1370)
* **Cut** (`LANG_0446`, L1731) `[Shortcut: Ctrl+X]`
* **Copy** (`LANG_0448`, L1735) `[Shortcut: Ctrl+C]`
* **Paste** (`LANG_0449`, L1743) *(Submenu)*
  * **Paste** (L1745) `[Shortcut: Ctrl+V]`
  * **Paste with origin** (`LANG_0450`, L1749)
  * **Paste with bspmodel** (L1753) `[Shortcut: Ctrl+V]`
  * **Paste at this origin** (L1757)
* **Delete** (`LANG_0451`, L1773) `[Shortcut: Del]`
* **Unhide** (`LANG_0453`, L1781) / **Hide** (`LANG_0455`, L1788) `[Shortcut: Ctrl+H]`
* **Hulls** (`LANG_0456`, L1801) *(Submenu)*
  * **Create Hull** (`LANG_0457`, L1805) -> *(Clipnodes, Hull 1..3)*
  * **Delete Hull** (`LANG_0459`, L1828) -> *(All Hulls, Clipnodes, Hull 1..3)*
  * **Simplify Hull** (`LANG_0461`, L1871) -> *(Clipnodes, Hull 1..3)*
  * **Redirect Hull** (`LANG_0462`, L1901) -> *(Hull i -> Hull k)*
  * **Print Hull Tree** (`LANG_0463`, L1931) -> *(Hull 1..3, Print HeadNodes)*
* **Duplicate BSP model** (L1976)
* **Unlock BSP model** (L2003)
* **MERGE BSPMODELS (WIP)** (L2049)
* **Export BSP model** (`LANG_0466`, L2103) *(Submenu)*
  * **With origin** (`LANG_0467`, L2105) -> *(With WAD, With internal textures [HL1], With internal textures [QUAKE/HL1+XASH])*
  * **Without origin** (`LANG_0471`, L2122) -> *(With WAD, With internal textures [HL1], With internal textures [QUAKE/HL1+XASH])*
* **Grab / Ungrab** (L2150) `[Shortcut: Alt+G]`
* **Transform** (`LANG_0474`, L2159) `[Shortcut: Ctrl+M]`
* **Properties** (`LANG_0476`, L2164) `[Shortcut: Alt+Enter]`

---

### 2.2. Face Context Menu (`face_context`, `Gui.cpp:L1408`)
* **Delete Faces** (`DELETE_FACES`, L1424)
* **Copy texture** (`LANG_0438`, L1436)
* **Paste texture** (`LANG_0440`, L1441) `[Shortcut: Ctrl+V]`
* **Copy Style** (`COPY_STYLE`, L1449)
* **Paste Style** (`PASTE_STYLE`, L1454)
* **Copy lightmap** (`LANG_0442`, L1461)
* **Paste lightmap** (`LANG_0445`, L1472)
* **Select same texture** (`SELECT_ALL_TEXTURED`, L1479)
* **Select model** (`SELECT_FACE_MDL`, L1507)
* **Select Linked** (L1532) *(Submenu: Depth 1..5)*
* **Export BSP model** (`LANG_0466`, L1608) *(Submenu: With origin, Without origin)*
* **Extract faces** (L1658)

---

### 2.3. Empty Viewport Context Menu (`empty_context`, `Gui.cpp:L2173`)
* **Paste** (L2177) `[Shortcut: Ctrl+V]`
* **Paste with origin** (`LANG_0450`, L2181)
* **Paste with bspmodel** (L2185) `[Shortcut: Ctrl+V]`

---

## 3. Structural Analysis & Identified UX Issues

1. **Massive Overcrowding in the `Map` Menu**:
   * The `Map` menu currently acts as a dumping ground containing 12 top-level submenus (`Additional tools`, `Cull actions`, `MAP TRANSFORMATION`, `Fixes`, `MDL to BSP`, `Delete Hull`, `Redirect Hull`, `Show clipnodes`, etc.).
   * Operational actions (mirroring/rotating map), utility fixers (bad surface extents), and deletion tools (hull management) are mixed together without functional separation.

2. **Fragmentation of Import / Export Options**:
   * Exporters are scattered between `File -> Export`, `File -> Save as` (BSP format choices), `Map -> Export BSP model`, `Viewport Context -> Export BSP model`, and standalone tools.
   * `File -> Save as` presents 13 individual BSP engine sub-formats in a flat, unorganized list.

3. **Inconsistent Naming and Status Badges**:
   * Experimental features are arbitrarily tagged with `[WIP]`, `(WIP)`, or `! (WIP)` in menu labels.
   * Non-standard menu labels (e.g. `Embedded##import`, `Options###1`, `PROTECT MAP!(WIP)`).

4. **Duplicate & Misplaced Interface Widgets**:
   * `Log` is listed twice in the `Widgets` menu (lines 6480 and 6527).
   * Key window toggles are split between `Widgets` and `Windows`.

5. **Cluttered Context Menus**:
   * Right-clicking an entity presents over 25 commands in a single vertical stack, mixing quick actions (Copy/Cut/Delete) with deep BSP structure modifications (Hull deletion, clipnode creation).
