/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2020  stiglers-eponym

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

#ifndef NAMES_H
#define NAMES_H

#include "enumerates.h"

/// Map tools to strings (used for saving to XML).
static const QMap<DrawTool, QString> toolNames = {
    {Highlighter, "highlighter"},
    {Pen, "pen"},
    {Magnifier, "magnifier"},
    {Torch, "torch"},
    {Pointer, "pointer"},
    {Eraser, "eraser"},
};

/// Default tools (FullDrawTool) for DrawTools
static const QMap<DrawTool, FullDrawTool> defaultToolConfig = {
    {Pen, {Pen, Qt::black, 3}},
    {Highlighter, {Highlighter, QColor(255,255,0,191), 30}},
    {Pointer, {Pointer, QColor(255,0,0,191), 12, {.pointer={128, 1, true}}}},
    {Magnifier, {Magnifier, Qt::gray, 200, {.magnification=2.}}},
    {Torch, {Torch, QColor(0,0,0,64), 120}},
    {Eraser, {Eraser, QColor(), 10.}},
    {NoTool, {NoTool, QColor(), 0.}},
    {InvalidTool, {InvalidTool, QColor(), 0.}},
};

/// Default tools (FullDrawTool) for KeyActions
static const QMap<KeyAction, DrawTool> actionToToolMap = {
    {DrawPen, Pen},
    {DrawHighlighter, Highlighter},
    {DrawPointer, Pointer},
    {DrawMagnifier, Magnifier},
    {DrawTorch, Torch},
};

/// Map KeyActions to names shown on the tool selector buttons.
static const QMap<KeyAction, QString> actionNames = {
    {Previous, "prev"},
    {Next, "next"},
    {PreviousNoTransition, "prev-no-transition"},
    {NextNoTransition, "next-no-transition"},
    {PreviousNotes, "prev (notes)"},
    {NextNotes, "next (notes)"},
    {PreviousSkippingOverlays, "prev slide"},
    {NextSkippingOverlays, "next slide"},
    {PreviousNotesSkippingOverlays, "prev slide (notes)"},
    {NextNotesSkippingOverlays, "ntext slide (notes)"},
    {GoToPage, "goto"},
    {LastPage, "last"},
    {FirstPage, "first"},
    {SyncFromControlScreen, "sync presentation"},
    {SyncFromPresentationScreen, "sync control"},
    {Update, "update"},
    {UpdateCache, "update cache"},

#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {StartEmbeddedCurrentSlide, "embedded"},
    {StartAllEmbedded, "all embedded"},
    {CloseEmbeddedCurrentSlide, "close embedded"},
    {CloseAllEmbedded, "close all embedded"},
#endif
    {PlayMultimedia, "play media"},
    {PauseMultimedia, "pause media"},
    {PlayPauseMultimedia, "play/pause"},

    {ToggleMuteAll, "togle mute"},
    {ToggleMuteNotes, "togle mute notes"},
    {ToggleMutePresentation, "toggle mute pres"},
    {MuteAll, "mute"},
    {MuteNotes, "mute notes"},
    {MutePresentation, "mute pres"},
    {UnmuteAll, "unmute"},
    {UnmuteNotes, "unmute notes"},
    {UnmutePresentation, "unmute pres"},

    {PlayPauseTimer, "timer"},
    {ContinueTimer, "continue timer"},
    {PauseTimer, "pause timer"},
    {ResetTimer, "reset timer"},
    {ToggleTOC, "TOC"},
    {ShowTOC, "TOC"},
    {ToggleOverview, "overview"},
    {ShowOverview, "overview"},
    {HideOverlays, "hide overlays"},
    {HideDrawSlide, "end drawing"},
    {ToggleCursor, "cursor"},
    {ShowCursor, "show cursor"},
    {HideCursor, "hide cursor"},
    {FullScreen, "full screen"},
    {Reload, "reload"},
    {Quit, "quit"},

    {ClearAnnotations, "clear"},
    {DrawNone, "hand"},
    {ToggleDrawMode, "draw"},
    {DrawTorch, "torch"},
    {DrawMagnifier, "magnifier"},
    {DrawMode, "draw"},
    {DrawEraser, "eraser"},
    {DrawPen, "pen"},
    {DrawHighlighter, "highlight"},
    {DrawPointer, "pointer"},
    {DrawMagnifier, "magnifier"},
    {DrawTorch, "torch"},

    {SaveDrawings, "save"},
    {LoadDrawings, "open"},
    {SaveDrawingsLegacy, "save legacy"},
    {SaveDrawingsUncompressed, "save uncompressed"},
    {SaveDrawingsXournal, "save xournal"},
};

/// Map KeyActions to icon names.
/// The icons are shown on the tool selector buttons.
static const QMap<KeyAction, QString> actionIconNames = {
    {PlayMultimedia, "media-playback-start"},
    {PauseMultimedia, "media-playback-stop"},
    {PlayPauseMultimedia, "media-playback-pause"},
    {Next, "go-next"},
    {Previous, "go-previous"},
    {FirstPage, "go-first"},
    {LastPage, "go-last"},
    {GoToPage, "go-jump"},
    {PreviousSkippingOverlays, "go-up"},
    {NextSkippingOverlays, "go-down"},
    {Quit, "application-exit"},
    {FullScreen, "view-fullscreen"},
    {Update, "view-refresh"},
    {ClearAnnotations, "edit-delete"},
    {PauseTimer, "media-playback-pause"},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {StartEmbeddedCurrentSlide, "application-x-executable"},
#endif
    {SaveDrawings, "document-save"},
    {LoadDrawings, "document-open"},
    {UndoDrawing, "edit-undo"},
    {RedoDrawing, "edit-redo"},
    {MuteAll, "audio-volume-muted"},
    {UnmuteAll, "audio-volume-high"},
    // TODO: more and better icons
};

/// Map draw tools to icon names.
/// The icons are shown on the tool selector buttons.
static const QMap<DrawTool, QString> toolIconNames = {
    // TODO: more and better icons
};

/// Map action strings in configuration file to KeyAction (enum)
static const QMap<QString, KeyAction> keyActionMap {
    {"previous", KeyAction::Previous},
    {"next", KeyAction::Next},
    {"previous no transition", KeyAction::PreviousNoTransition},
    {"next no transition", KeyAction::NextNoTransition},
    {"previous notes", KeyAction::PreviousNotes},
    {"next notes", KeyAction::NextNotes},
    {"previous skipping overlays", KeyAction::PreviousSkippingOverlays},
    {"next skipping overlays", KeyAction::NextSkippingOverlays},
    {"previous notes skipping overlays", KeyAction::PreviousNotesSkippingOverlays},
    {"next notes skipping overlays", KeyAction::NextNotesSkippingOverlays},
    {"go to", KeyAction::GoToPage},
    {"go to page", KeyAction::GoToPage},
    {"go to slide", KeyAction::GoToPage},
    {"last page", KeyAction::LastPage},
    {"last slide", KeyAction::LastPage},
    {"first page", KeyAction::FirstPage},
    {"first slide", KeyAction::FirstPage},
    {"sync from control screen", KeyAction::SyncFromControlScreen},
    {"sync from presentation screen", KeyAction::SyncFromPresentationScreen},
    {"update", KeyAction::Update},

    {"update cache", KeyAction::UpdateCache},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {"start embedded current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start all embedded", KeyAction::StartAllEmbedded},
    {"start all embedded applications", KeyAction::StartAllEmbedded},
    {"close all embedded", KeyAction::CloseAllEmbedded},
    {"close all embedded applications", KeyAction::CloseAllEmbedded},
    {"close embedded current page", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded current slide", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded applications current page", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded applications current slide", KeyAction::CloseEmbeddedCurrentSlide},
#endif
    {"play multimedia", KeyAction::PlayMultimedia},
    {"pause multimedia", KeyAction::PauseMultimedia},
    {"play pause multimedia", KeyAction::PlayPauseMultimedia},
    {"toggle multimedia", KeyAction::PlayPauseMultimedia},

    {"toggle mute",              KeyAction::ToggleMuteAll},
    {"toggle mute presentation", KeyAction::ToggleMutePresentation},
    {"toggle mute notes",        KeyAction::ToggleMuteNotes},
    {"mute",              KeyAction::MuteAll},
    {"mute all",          KeyAction::MuteAll},
    {"mute presentation", KeyAction::MutePresentation},
    {"mute notes",        KeyAction::MuteNotes},
    {"unmute",              KeyAction::UnmuteAll},
    {"unmute all",          KeyAction::UnmuteAll},
    {"unmute presentation", KeyAction::UnmutePresentation},
    {"unmute notes",        KeyAction::UnmuteNotes},

    {"toggle timer", KeyAction::PlayPauseTimer},
    {"pause timer", KeyAction::PauseTimer},
    {"continue timer", KeyAction::ContinueTimer},
    {"reset timer", KeyAction::ResetTimer},
    {"toggle toc", KeyAction::ToggleTOC},
    {"show toc", KeyAction::ShowTOC},
    {"show overview", KeyAction::ShowOverview},
    {"toggle overview", KeyAction::ToggleOverview},
    {"hide overlays", KeyAction::HideOverlays},
    {"show notes", KeyAction::HideOverlays},
    {"toggle cursor", KeyAction::ToggleCursor},
    {"show cursor", KeyAction::ShowCursor},
    {"hide cursor", KeyAction::HideCursor},
    {"full screen", KeyAction::FullScreen},
    {"reload", KeyAction::Reload},
    {"quit", KeyAction::Quit},

    {"clear annotations", KeyAction::ClearAnnotations},
    {"hand tool", KeyAction::DrawNone},
    {"none", KeyAction::DrawNone},
    {"pointer", KeyAction::DrawPointer},
    {"pen", KeyAction::DrawPen},
    {"highlighter", KeyAction::DrawHighlighter},
    {"torch", KeyAction::DrawTorch},
    {"magnifier", KeyAction::DrawMagnifier},
    {"eraser", KeyAction::DrawEraser},

    {"toggle draw mode", KeyAction::ToggleDrawMode},
    {"enter draw mode", KeyAction::DrawMode},
    {"draw mode", KeyAction::DrawMode},
    {"hide draw slide", KeyAction::HideDrawSlide},
    {"end draw mode", KeyAction::HideDrawSlide},
    {"end drawing", KeyAction::HideDrawSlide},
    {"undo drawing", KeyAction::UndoDrawing},
    {"redo drawing", KeyAction::RedoDrawing},
    {"undo", KeyAction::UndoDrawing},
    {"redo", KeyAction::RedoDrawing},
    {"save drawings", KeyAction::SaveDrawings},
    {"save drawings xournal", KeyAction::SaveDrawingsXournal},
    {"save drawings xournal++", KeyAction::SaveDrawingsXournal},
    {"save drawings compatibility", KeyAction::SaveDrawingsXournal},
    {"load drawings", KeyAction::LoadDrawings},
    {"save drawings legacy", KeyAction::SaveDrawingsLegacy},
    {"save drawings uncompressed", KeyAction::SaveDrawingsUncompressed},
    {"save", KeyAction::SaveDrawings},
    {"save xournal", KeyAction::SaveDrawingsXournal},
    {"save xournal++", KeyAction::SaveDrawingsXournal},
    {"save compatibility", KeyAction::SaveDrawingsXournal},
    {"load", KeyAction::LoadDrawings},
    {"save legacy", KeyAction::SaveDrawingsLegacy},
    {"save uncompressed", KeyAction::SaveDrawingsUncompressed},
};

/// Map tool strings from configuration file to DrawTool (enum).
/// Every tool string defined here must also be a valid key action string in keyActionMap.
static const QMap<QString, DrawTool> toolMap {
    {"pen", DrawTool::Pen},
    {"highlighter", DrawTool::Highlighter},
    {"none", DrawTool::NoTool},
    {"pointer", DrawTool::Pointer},
    {"eraser", DrawTool::Eraser},
    {"torch", DrawTool::Torch},
    {"magnifier", DrawTool::Magnifier},
};

/// Default mapping of keys to KeyAction actions.
/// This is used if the configuration file does not define any key mapping.
static const QMap<quint32, QList<KeyAction>> defaultKeyMap = {
    {Qt::Key_PageUp, {KeyAction::Previous}},
    {Qt::Key_PageDown, {KeyAction::Next}},
    {Qt::Key_Left, {KeyAction::Previous}},
    {Qt::Key_Right, {KeyAction::Next}},
    {Qt::Key_Up, {KeyAction::PreviousSkippingOverlays}},
    {Qt::Key_Down, {KeyAction::NextSkippingOverlays}},
    {Qt::Key_G, {KeyAction::GoToPage}},
    {Qt::Key_End, {KeyAction::LastPage}},
    {Qt::Key_Home, {KeyAction::FirstPage}},
    {Qt::Key_Return, {KeyAction::SyncFromControlScreen}},
    {Qt::Key_Escape, {KeyAction::SyncFromPresentationScreen, KeyAction::HideOverlays, KeyAction::HideDrawSlide}},
    {Qt::Key_Space, {KeyAction::Update}},

    {Qt::Key_C, {KeyAction::UpdateCache}},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {Qt::Key_E, {KeyAction::StartEmbeddedCurrentSlide}},
    {Qt::Key_E+Qt::ShiftModifier, {KeyAction::StartAllEmbedded}},
#endif
    {Qt::Key_M, {KeyAction::PlayPauseMultimedia}},

    {Qt::Key_P, {KeyAction::PlayPauseTimer}},
    {Qt::Key_R, {KeyAction::ResetTimer}},
    {Qt::Key_T, {KeyAction::ShowTOC}},
    {Qt::Key_S, {KeyAction::ShowOverview}},
    {Qt::Key_O, {KeyAction::ToggleCursor}},
    {Qt::Key_F, {KeyAction::FullScreen}},
    {Qt::Key_U, {KeyAction::Reload}},
    {Qt::Key_Q+Qt::CTRL, {KeyAction::Quit}},
    {Qt::Key_Z+Qt::CTRL, {KeyAction::UndoDrawing}},
    {Qt::Key_Y+Qt::CTRL, {KeyAction::RedoDrawing}},
};

/// Map of keys to KeyActions for hard coded key bindings.
/// The keys in this list are used in special modes (TOC and overview mode).
static const QMap<quint32, KeyAction> staticKeyMap = {
    {Qt::Key_Left, KeyAction::Left},
    {Qt::Key_Right, KeyAction::Right},
    {Qt::Key_Up, KeyAction::Up},
    {Qt::Key_Down, KeyAction::Down},
    {Qt::Key_End, KeyAction::End},
    {Qt::Key_Home, KeyAction::First},
    {Qt::Key_Return, KeyAction::Return},
    {Qt::Key_Tab, KeyAction::Tab},
    {Qt::Key_Tab+Qt::SHIFT, KeyAction::ShiftTab},
};

/// Default configuration of actions for tool selector (buttons).
/// The tool selector is an array of buttons in the lower right corner of the control screen.
/// The keys (quint8) in this map are interpreted as two digit hexadecimal numbers, where the first digit defines the row and the second one defines the column of the button in the array.
/// One button can define several KeyActions.
static const QMap<quint8, QList<KeyAction>> defaultToolSelectorActions {
    {0, {KeyAction::ToggleDrawMode}},
    {1, {KeyAction::DrawEraser}},
    {16, {KeyAction::PlayMultimedia}},
    {17, {KeyAction::ClearAnnotations}},
    {19, {KeyAction::DrawTorch}},
    {20, {KeyAction::DrawMagnifier}},
};

/// Default configuration of tools for tool selector (buttons).
/// The tool selector is an array of buttons in the lower right corner of the control screen.
/// The keys (quint8) in this map are interpreted as two digit hexadecimal numbers, where the first digit defines the row and the second one defines the column of the button in the array.
static const QMap<quint8, FullDrawTool> defaultToolSelectorTools {
    {2, {Pen, QColor("red"), 3.}},
    {3, {Pen, QColor("green"), 3.}},
    {4, {Highlighter, QColor(255,255,0,191), 30}},
    {18, {Pointer, QColor(255,0,0,191), 12, {.pointer={128, 1, true}}}},
};

#endif // NAMES_H
