# Proposed Reordered Menu Structure (English)

## 1. Main Menu Bar
### File
- **Open…** – open a BSP, MAP, or other supported file.
- **Open Recent** – shows recent files (current implementation uses `Recent Files`).
- **Save As…** – export the current map (`LANG_0479`).
- **Export** – submenu containing all export formats:
  - **Export .obj** (`LANG_3328`)
  - **Export .csm** (`LANG_3343`)
  - **Export Valve Hammer (.map) [WIP]** (`LANG_3371`)
  - **Export StudioModel (.smd) [WIP]** (`LANG_3261`)
  - **Export UnrealMapDrawTool (.umd) [WIP]** (`LANG_3519`)
  - **Export with/without palette** (options `LANG_0481` / `LANG_0482`).
- **Import Textures** – moved under *File* (currently handled by the PNG import dialog).
- **Close** – close current map.

### Edit
- **Model Origin / Position** (formerly under the context menu):
  - **Set to Center** – sets origin to model centre.
  - **Set X/Y/Z individually** – options `LANG_0432`‑`LANG_0437`.
- **Select Linked** – depth‑controlled selection submenu.
- **Extract Faces** – extracts selected faces to a new model.
- **Hull #** – dynamic submenu for hull selection.
- **Scale** – submenu for scaling operations (`Select scale`).

### View
- **Toggle Dithering** – enable/disable texture dithering.
- **Show/Hide Grid**, **Show/Hide Axes**, etc. (placeholders for future UI features).

### Tools (new top‑level menu)
- **Additional Tools** – group all utility actions:
  - **Delete OOB Data**
  - **Fix Bad Surface Extents**
  - **MDL → BSP (WIP)**
  - **Copy / Paste Style**
  - **Copy / Paste Lightmap**
  - **Copy / Paste Texture**
  - **Copy / Paste Scale**
- **Options** – move advanced options here:
  - **[Scan] Cell size**
  - **Embedded Import** (`Embedded##import`)
  - **Advanced Export Settings** (palette, blue shift, etc.)

### Help
- **About**, **Help**, **Keyboard Shortcuts** – standard help entries (not yet implemented but reserved).

## Rationale
- **Logical grouping** – actions that affect the whole map live under *File* or *Edit*; low‑level utilities are collected under a dedicated *Tools* menu.
- **Reduced clutter** – the original implementation mixed context‑specific actions with top‑level menus, making the UI feel disorganized.
- **Future extensibility** – placeholders for view toggles and help make it easier to extend the UI without further re‑ordering.

> This proposed structure mirrors common desktop‑application conventions (File → Edit → View → Tools → Help) while preserving all existing functionalities.
