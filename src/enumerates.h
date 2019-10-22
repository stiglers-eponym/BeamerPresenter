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

#include<QColor>

enum PagePart {
    LeftHalf = 1,
    FullPage = 0,
    RightHalf = -1,
};

enum KeyAction {
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
    PlayMultimedia,

    PauseTimer,
    ResetTimer,
    ShowTOC,
    HideTOC,
    ShowOverview,
    HideOverview,
    HideDrawSlide,
    ToggleCursor,
    FullScreen,
    Reload,
    Quit,

    ClearAnnotations,
    DrawNone,
    DrawTorch,
    DrawMagnifier,
    DrawMode,
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

#endif // ENUMERATES_H
