# Original Menu Structure (extracted from src/editor/Gui.cpp)

## Main menu bar
- **FILE** (`LANG_0478`) – primary menu (likely "File")
  - **Save As…** (`LANG_0479`) – line 2366
  - **Export** (`LANG_0480`) – line 2373
    - Sub‑options (lines 2386‑2408) such as "No palette", "Blue Shift", etc.
- **Export** (`LANG_0480`) – appears again as a top‑level menu (line 2373)
- **Options** (`LANG_0478`) – contains sub‑menus and BSP format options.

## Contextual menus (drawBspContexMenu and related)
- **Model Origin / Position** (`LANG_0431`) – line 1216
  - Items: `LANG_0432`, `LANG_0433`, `LANG_0434`, `LANG_0435`, `LANG_0436`, `LANG_0437` (lines 1218‑1366) – ways to set the model's origin on different axes.
- **Set Origin** (`LANG_0430`) – line 1208 – a direct menu item.
- **Select Linked** – line 1532 – submenu offering depth selections (Depth 1‑5, lines 1534‑1549).
- **Extract faces** – line 1658 – direct action.
- **Hull "#"** – line 1905 – dynamic submenu listing hull numbers (lines 1905‑1910).
- **Options###1** – line 3527 – advanced options submenu.
  - **[Scan] Cell size** – line 3529 – option inside Options###1.
- **Recent Files** – line 4493 – shows recently opened files.
- **Additional tools** – line 4995 – submenu with utility actions:
  - **Delete OOB Data** – line 4997
  - **Fix Bad Surface Extents** – line 5065
  - **MDL to BSP (WIP)** – line 4809
  - … and several others.

## Other detected menus
- **Select scale** – line 3288
- **Export .obj** – line 3328
- **Export .csm** – line 3343
- **ValveHammerEditor (.map) [WIP]** – line 3371
- **StudioModel Data (.smd) [WIP]** – line 3261
- **Wavefront(.obj)/XashNT(.csm) [WIP]** – line 3286
- **UnrealMapDrawTool (.umd) [WIP]** – line 3519
- **Embedded##import** – line 4398

> **Note:** Menu names retrieved via `get_localized_string(LANG_XXXX)` depend on localisation files; identifiers are shown to keep the mapping clear.
