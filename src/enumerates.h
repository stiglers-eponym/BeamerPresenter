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

enum PagePart {
    LeftHalf = 1,
    FullPage = 0,
    RightHalf = -1,
};

enum KeyAction {
    NoAction,

    Previous,
    Next,
    PreviousNotes,
    NextNotes,
    PreviousSkippingOverlays,
    NextSkippingOverlays,
    PreviousNotesSkippingOverlays,
    NextNotesSkippingOverlays,
    GoToPage,
    LastPage,
    FirstPage,
    SyncFromControlScreen,
    SyncFromPresentationScreen,
    Update,
    UpdateCache,

#ifdef EMBEDDED_APPLICATIONS_ENABLED
    StartEmbeddedCurrentSlide,
    StartAllEmbedded,
    CloseEmbeddedCurrentSlide,
    CloseAllEmbedded,
#endif
    PlayPauseMultimedia,
    PlayMultimedia,
    PauseMultimedia,

    PlayPauseTimer,
    ContinueTimer,
    PauseTimer,
    ResetTimer,
    ToggleTOC,
    ShowTOC,
    HideTOC,
    ToggleOverview,
    ShowOverview,
    HideOverview,
    HideDrawSlide,
    ToggleCursor,
    ShowCursor,
    HideCursor,
    FullScreen,
    Reload,
    Quit,

    ClearAnnotations,
    DrawNone,
    DrawTorch,
    DrawMagnifier,
    DrawMode,
    ToggleDrawMode,
    DrawPen,
    DrawEraser,
    DrawHighlighter,
    DrawPointer,
};

enum DrawTool {
    None,
    Highlighter,
    Pen,
    Pointer,
    Torch,
    Eraser,
    Magnifier,
};

struct ColoredDrawTool {
    DrawTool tool;
    QColor color;
};

// Default tools for key actions:
static const QMap<KeyAction, ColoredDrawTool> actionToToolMap = {
    {DrawPen, {Pen,Qt::black}},
    {DrawHighlighter, {Highlighter,QColor(255,255,0,191)}},
    {DrawPointer, {Pointer,QColor(255,0,0,191)}},
    {DrawMagnifier, {Magnifier, QColor(64,64,64,64)}},
    {DrawTorch, {Torch, QColor(0,0,0,64)}},
};

#endif // ENUMERATES_H
