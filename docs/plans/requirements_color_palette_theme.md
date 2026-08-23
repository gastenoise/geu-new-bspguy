# Requirements Document: Application-Wide Color Palette Implementation

## Introduction

The goal of this project is to apply a comprehensive, visually cohesive Gothic Dark theme to the entire user interface of `revamped-newbspguy`. The application currently relies on standard default ImGui dark styling (`ImGui::StyleColorsDark()`) alongside fragmented hardcoded inline HSV and RGB values across various dialogs and custom widgets.

This document defines the functional and non-functional requirements to establish a unified color palette throughout all UI elements, windows, dialogs, viewport overlays, and file dialogs.

---

## Target Color Palette Specification

| Color Name | Hex Code | RGB (0-255) | Normalized Float (RGBA) | Designated UI Role |
| :--- | :--- | :--- | :--- | :--- |
| **Deep Obsidian** | `#0B0C10` | (11, 12, 16) | `(0.043f, 0.047f, 0.063f, 1.00f)` | Window backgrounds, child windows, popup backgrounds, menu bar base |
| **Blood Crimson** | `#8B142A` | (139, 20, 42) | `(0.545f, 0.078f, 0.165f, 1.00f)` | Primary accents, active buttons, active tabs, highlights, slider grab active |
| **Nightmare Purple** | `#26294A` | (38, 41, 74) | `(0.149f, 0.161f, 0.290f, 1.00f)` | Secondary background containers, frame backgrounds, inactive buttons/tabs, headers |
| **Gargoyle Stone Grey** | `#6E7A86` | (110, 122, 134) | `(0.431f, 0.478f, 0.525f, 1.00f)` | Borders, separators, disabled text, scrollbar grabs, subtle structural lines |
| **Vellum Cream** | `#E3D5B8` | (227, 213, 184) | `(0.890f, 0.835f, 0.722f, 1.00f)` | Primary text, high-visibility icons, selected item text, active cursor indicators |

---

## Glossary

- **ImGui**: Immediate Mode Graphical User Interface library used for rendering all editor windows and dialogs.
- **ImGuiCol**: Enumeration defining color targets for standard ImGui elements (e.g. `ImGuiCol_WindowBg`, `ImGuiCol_ButtonActive`).
- **ImVec4**: 4-component floating-point vector used by ImGui to specify RGBA colors in normalized range `[0.0, 1.0]`.
- **Pick Mode**: Mode selecting objects, faces, or leaf nodes in the 3D viewport.
- **ImFileDialog**: Custom file chooser modal dialog integrated into the editor.

---

## Requirements

### Requirement 1: Centralized Theme Initialization
**User Story:** As a developer, I want a centralized theme setup function so that the color palette is applied consistently at startup and easy to maintain.

#### Acceptance Criteria
1. THE system SHALL define a centralized theme setup method `Gui::setupTheme()` invoked during GUI initialization in `Gui::init()`.
2. THE system SHALL replace the default `ImGui::StyleColorsDark()` call with `setupTheme()`.
3. THE system SHALL define reusable palette color constants (`COLOR_DEEP_OBSIDIAN`, `COLOR_BLOOD_CRIMSON`, `COLOR_NIGHTMARE_PURPLE`, `COLOR_GARGOYLE_GREY`, `COLOR_VELLUM_CREAM`) in `Gui.h` or a dedicated theme header.

---

### Requirement 2: Complete ImGui Style Mapping
**User Story:** As a user, I want all standard ImGui widgets (buttons, frames, text, scrollbars, headers, tabs, menus) to adhere to the target color palette.

#### Acceptance Criteria
1. THE system SHALL map `ImGuiCol_WindowBg`, `ImGuiCol_ChildBg`, `ImGuiCol_PopupBg`, `ImGuiCol_TitleBg`, and `ImGuiCol_MenuBarBg` to **Deep Obsidian** (`#0B0C10`).
2. THE system SHALL map `ImGuiCol_FrameBg`, `ImGuiCol_Header`, `ImGuiCol_Button`, and `ImGuiCol_Tab` to **Nightmare Purple** (`#26294A`).
3. THE system SHALL map `ImGuiCol_ButtonActive`, `ImGuiCol_HeaderActive`, `ImGuiCol_TabActive`, `ImGuiCol_CheckMark`, `ImGuiCol_SliderGrabActive`, and `ImGuiCol_TitleBgActive` to **Blood Crimson** (`#8B142A`).
4. THE system SHALL map `ImGuiCol_Border`, `ImGuiCol_Separator`, `ImGuiCol_ScrollbarGrab`, `ImGuiCol_SliderGrab`, and `ImGuiCol_TextDisabled` to **Gargoyle Stone Grey** (`#6E7A86`).
5. THE system SHALL map `ImGuiCol_Text` to **Vellum Cream** (`#E3D5B8`).

---

### Requirement 3: Refactoring Hardcoded Inline Colors in Dialogs
**User Story:** As an editor user, I want dialog windows (such as Entity Report, Merge Wizard, Transform, and Settings) to follow the theme instead of using jarring hardcoded red or bright HSV colors.

#### Acceptance Criteria
1. WHEN dialogs render buttons, headers, or alerts, THE system SHALL use theme constants instead of hardcoded `ImColor::HSV(0, 0.6f, 0.6f)` or raw RGB vectors.
2. IF a dialog requires warning or confirm action styling, THEN THE system SHALL use **Blood Crimson** (`#8B142A`) for primary actions and **Nightmare Purple** (`#26294A`) for neutral buttons.

---

### Requirement 4: Viewport Overlay & Pick Mode Color Harmonization
**User Story:** As a mapper, I want the viewport pick mode selection buttons (Object, Face, Leaf) and 3D overlay text to visually align with the theme.

#### Acceptance Criteria
1. THE system SHALL style active pick mode buttons using **Blood Crimson** (`#8B142A`) and inactive pick mode buttons using **Nightmare Purple** (`#26294A`).
2. THE system SHALL render pick mode button borders using **Gargoyle Stone Grey** (`#6E7A86`) / **Vellum Cream** (`#E3D5B8`).
3. THE 3D viewport context text and status bar overlays SHALL maintain crisp contrast using **Vellum Cream** (`#E3D5B8`) text.

---

### Requirement 5: File Dialog Styling Consistency
**User Story:** As a mapper opening or saving files, I want the file dialog (`ImFileDialog`) to inherit the target palette so it does not look like a mismatched external tool.

#### Acceptance Criteria
1. THE file dialog SHALL automatically utilize the active `ImGuiStyle` colors (`ImGuiCol_Header`, `ImGuiCol_HeaderActive`, `ImGuiCol_Text`, `ImGuiCol_FrameBg`).
2. ANY custom drawn items in `ImFileDialog.cpp` SHALL reference `ImGui::GetStyle().Colors[...]` backed by the active palette.

---

### Requirement 6: Contrast & Accessibility Standards
**User Story:** As a user working long editing sessions, I want high text readability and smooth color contrasts so that eye strain is minimized.

#### Acceptance Criteria
1. THE system SHALL ensure all primary text (**Vellum Cream**) over window and container backgrounds (**Deep Obsidian**, **Nightmare Purple**, **Blood Crimson**) achieves a contrast ratio meeting or exceeding WCAG 2.1 AA (at least 4.5:1).
2. Hover and active interaction states SHALL provide clear visual feedback distinguishable within 100ms of user input.
