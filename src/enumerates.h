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
    StartEmbeddedCurrentSlide,
    StartAllEmbedded,
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
    DrawRedPen,
    DrawGreenPen,
    DrawHighlighter,
    DrawTorch,
    DrawPointer,
    DrawMagnifier,
    DrawMode,
};

enum DrawTool {
    None,
    Highlighter,
    RedPen,
    GreenPen,
    Pointer,
    Torch,
    Eraser,
    Magnifier,
};

#endif // ENUMERATES_H
