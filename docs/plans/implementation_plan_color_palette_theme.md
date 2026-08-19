# Implementation Plan: Gothic Dark Color Palette Application

This implementation plan outlines the step-by-step task breakdown for applying the 5-color Gothic Dark palette across `revamped-newbspguy`.

---

## Task Hierarchy & Checklists

- [ ] 1. Architecture & Central Theme Infrastructure Setup
  - [ ] 1.1 Declare theme constants and `setupTheme()` function header in `src/editor/Gui.h`.
    - _Requirements: REQ-1.1, REQ-1.3_
    - _Dependencies: None_
  - [ ] 1.2 Implement `setupTheme()` in `src/editor/Gui.cpp` mapping all 55+ `ImGuiCol_` slots to the target palette.
    - _Requirements: REQ-1.1, REQ-2.1, REQ-2.2, REQ-2.3, REQ-2.4, REQ-2.5_
    - _Dependencies: Task 1.1_
  - [ ] 1.3 Replace default `ImGui::StyleColorsDark()` with `setupTheme()` in `Gui::init()`.
    - _Requirements: REQ-1.2_
    - _Dependencies: Task 1.2_

- [ ] 2. Dialogs & Custom Widgets Color Refactoring
  - [ ] 2.1 Refactor hardcoded HSV button colors in `src/editor/gui/GuiDialogs.cpp` to use `COLOR_BLOOD_CRIMSON` and `COLOR_NIGHTMARE_PURPLE`.
    - _Requirements: REQ-3.1, REQ-3.2_
    - _Dependencies: Phase 1_
  - [ ] 2.2 Refactor pick mode toggle button colors (Object, Face, Leaf) in `src/editor/Gui.cpp` to align with `COLOR_BLOOD_CRIMSON` and `COLOR_NIGHTMARE_PURPLE`.
    - _Requirements: REQ-4.1, REQ-4.2_
    - _Dependencies: Phase 1_
  - [ ] 2.3 Refactor custom widgets and hull indicators in `src/editor/gui/GuiWidgets.cpp` to use theme palette tokens.
    - _Requirements: REQ-3.1, REQ-4.3_
    - _Dependencies: Phase 1_

- [ ] 3. File Dialog & Viewport HUD Integration
  - [ ] 3.1 Verify `src/filedialog/ImFileDialog.cpp` draws headers, text, and selections using `ImGui::GetStyle().Colors`.
    - _Requirements: REQ-5.1, REQ-5.2_
    - _Dependencies: Phase 1_
  - [ ] 3.2 Audit 3D viewport context text and status bar overlays for color consistency and high contrast legibility.
    - _Requirements: REQ-4.3, REQ-6.1, REQ-6.2_
    - _Dependencies: Phase 2_

- [ ] 4. Build Verification & Visual Audit
  - [ ] 4.1 Execute full CMake compilation and check for zero build warnings or syntax errors.
    - _Requirements: REQ-1.1, REQ-6.1_
    - _Dependencies: Phase 3_
  - [ ] 4.2 Run runtime UI audit across menu bar, dialogs, viewport, pick modes, and settings window.
    - _Requirements: REQ-2.1 to REQ-6.2_
    - _Dependencies: Task 4.1_
