/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QColor>
#include <QMap>

/// Page part:
/// Choose that only half of each pdf page contains the presentation slide, while the other one contains a notes slide.
enum PagePart {
    /// presentation is shown on the left half.
    LeftHalf = 1,
    /// default: pdf contains only presentation.
    FullPage = 0,
    /// presentation is shown on the right half.
    RightHalf = -1,
};

/// KeyAction: Actions handled by ControlScreen
enum KeyAction {
    /// No Key Action. Used to indicate errors and missing KeyActions.
    NoAction,

    /// Go to the previous slide on presentation and control screen.
    Previous,
    /// Go to the next slide on presentation and control screen.
    Next,
    /// Go to the previous slide on presentation and control screen without a slide transition.
    PreviousNoTransition,
    /// Go to the next slide on presentation and control screen without a slide transition.
    NextNoTransition,
    /// Go to the previous slide only on the control screen.
    PreviousNotes,
    /// Go to the next slide only on the control screen.
    NextNotes,
    /// Go to the previous slide with a different page label on presentation and control screen.
    PreviousSkippingOverlays,
    /// Go to the next slide with a different page label on presentation and control screen.
    NextSkippingOverlays,
    /// Go to the previous slide with a different page label only on the control screen.
    PreviousNotesSkippingOverlays,
    /// Go to the next slide with a different page label only on the control screen.
    NextNotesSkippingOverlays,
    /// Go to the page label editor.
    GoToPage,
    /// Go to the last page on presentation and control screen.
    LastPage,
    /// Go to the first page on presentation and control screen.
    FirstPage,
    /// Adapt the slide on the presentation screen to the currently shown slide on the control screen.
    SyncFromControlScreen,
    /// Adapt the slide on the control screen to the currently shown slide on the presentation screen.
    SyncFromPresentationScreen,
    /// Update the currently shown slide.
    Update,
    /// Update the cache.
    UpdateCache,

#ifdef EMBEDDED_APPLICATIONS_ENABLED
    /// Start all embedded applications on the currently shown slide.
    StartEmbeddedCurrentSlide,
    /// Start all embedded applications on all slides.
    StartAllEmbedded,
    /// Close all embedded applications on the currently shown slide.
    CloseEmbeddedCurrentSlide,
    /// Close all embedded applications on all slides.
    CloseAllEmbedded,
#endif
    /// Toggle play/pause all multimedia content of the current slide.
    PlayPauseMultimedia,
    /// Play all multimedia content of the current slide.
    PlayMultimedia,
    /// Pause all multimedia content of the current slide.
    PauseMultimedia,

    /// Toggle mute/unmute all multimedia content.
    ToggleMuteAll,
    /// Toggle mute/unmute all multimedia content on the control screen.
    ToggleMuteNotes,
    /// Toggle mute/unmute all multimedia content on the presentation screen.
    ToggleMutePresentation,
    /// Mute all multimedia content.
    MuteAll,
    /// Mute all multimedia content on the control screen.
    MuteNotes,
    /// Mute all multimedia content on the presentation screen.
    MutePresentation,
    /// Unmute all multimedia content.
    UnmuteAll,
    /// Unmute all multimedia content on the control screen.
    UnmuteNotes,
    /// Unmute all multimedia content on the presentation screen.
    UnmutePresentation,

    /// Toggle start/stop timer.
    PlayPauseTimer,
    /// Continue timer.
    ContinueTimer,
    /// Pause timer.
    PauseTimer,
    /// Reset timer.
    ResetTimer,
    /// Show/hide table of contents.
    ToggleTOC,
    /// Show table of contents.
    ShowTOC,
    /// Show/hide overview (thumbnail slides).
    ToggleOverview,
    /// Hide TOC and Overview, show notes or draw slide again.
    HideOverlays,
    /// Show overview (thumbnail slides).
    ShowOverview,
    /// Hide draw slide (return to notes if separate notes are given).
    HideDrawSlide,
    /// Show/hide mouse cursor.
    ToggleCursor,
    /// Show mouse cursor.
    ShowCursor,
    /// Hide mouse cursor.
    HideCursor,
    /// Toggle full screen.
    FullScreen,
    /// Reload pdf files.
    Reload,
    /// Quit BeamerPresenter.
    Quit,

    /// Clear all annotations (drawings) on the current slide.
    ClearAnnotations,
    /// Set draw tool to "NoTool", i.e. disable drawing but do not leave draw mode.
    DrawNone,
    /// Set draw tool to torch.
    DrawTorch,
    /// Set draw tool to magnifier.
    DrawMagnifier,
    /// Enter draw mode. In this mode the presentation slide is shown on the control screen instead of the notes slide.
    DrawMode,
    /// Enter/quit draw mode.
    ToggleDrawMode,
    /// Set draw tool to pen (with default color).
    DrawPen,
    /// Set draw tool to eraser.
    DrawEraser,
    /// Set draw tool to highlighter (with default color).
    DrawHighlighter,
    /// Set draw tool to pointer (with default color).
    DrawPointer,
    /// Undo the last stroke (does not undo eraser!).
    UndoDrawing,
    /// Restore the latest deleted stroke.
    RedoDrawing,

    /// Save drawings to compressed XML file.
    SaveDrawings,
    /// Save drawings to uncompressed XML file.
    SaveDrawingsUncompressed,
    /// Load drawings from compressed or uncompressed XML file.
    LoadDrawings,
    /// Save drawings to legacy binary file (deprecated!)
    SaveDrawingsLegacy,
    /// Save drawings to Xournal(++) compatibility XML format
    SaveDrawingsXournal,

    // Hard coded keys used to directly pass raw key events.
    // Arrow keys
    /// Arrow key down
    Down,
    /// Arrow key up
    Up,
    /// Arrow key left
    Left,
    /// Arrow key right
    Right,
    // Navigation keys
    /// Key end
    End,
    /// Key first (home)
    First,
    // Other important keys
    /// Key return
    Return,
    /// Key tab
    Tab,
    /// Key combination shift+tab
    ShiftTab,
};

/// Tools used for drawing and highlighting
enum DrawTool {
    /// No draw tool.
    NoTool,
    /// Invalid draw tool (used to indicates errors).
    InvalidTool,
    /// Highlighter: wide, transparent, round pen which only darkens the background slide.
    Highlighter,
    /// Pen: usually non-transparent pen drawing over the background slide.
    Pen,
    /// Pointer: Small laser pointer, which only darkens the background.
    Pointer,
    /// Torch: Darken the slide except inside a circle around the cursor.
    Torch,
    /// Eraser: erase strokes of pens and highlighters.
    Eraser,
    /// Magnifier: zoom in a circular area around the cursor.
    Magnifier,
};

// TODO: remove this.
/// External or internal renderer.
enum Renderer {
    RenderPoppler = 0,
    RenderCustom = 1,
};

/// DrawTool combined with a QColor and a size.
struct FullDrawTool {
    DrawTool tool;
    QColor color;
    qreal size;
    union {
        qreal magnification;
        struct {
            quint16 alpha;
            /// 1 for darken, -1 for lighten, 0 for source over
            qint8 composition;
            bool inner;
        } pointer;
    } extras = {0.};
};

#endif // ENUMERATES_H
