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
    PreviousCurrentScreen,
    NextCurrentScreen,
    PreviousSkippingOverlays,
    NextSkippingOverlays,
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
    ToggleCursor,
    FullScreen,
    Reload,
    Quit
};

#endif // ENUMERATES_H
