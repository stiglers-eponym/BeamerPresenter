// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef NAMES_H
#define NAMES_H

#include <QMap>
#include <QString>

#include "src/config.h"
#include "src/enumerates.h"

/// Convert strings to GuiWidget
GuiWidget string_to_widget_type(const QString &string) noexcept;

/// Translate strings appearing in config to Actions
static const QMap<QString, Action> string_to_action_map{
    // Nagivation actions
    {QT_TRANSLATE_NOOP("SettingsWidget", "update"), Update},
    {QT_TRANSLATE_NOOP("SettingsWidget", "next"), NextPage},
    {QT_TRANSLATE_NOOP("SettingsWidget", "previous"), PreviousPage},
    {QT_TRANSLATE_NOOP("SettingsWidget", "next skipping overlays"),
     NextSkippingOverlays},
    {QT_TRANSLATE_NOOP("SettingsWidget", "previous skipping overlays"),
     PreviousSkippingOverlays},
    {QT_TRANSLATE_NOOP("SettingsWidget", "first"), FirstPage},
    {QT_TRANSLATE_NOOP("SettingsWidget", "last"), LastPage},
    // Drawing
    {QT_TRANSLATE_NOOP("SettingsWidget", "undo"), UndoDrawing},
    {QT_TRANSLATE_NOOP("SettingsWidget", "undo left"), UndoDrawingLeft},
    {QT_TRANSLATE_NOOP("SettingsWidget", "undo right"), UndoDrawingRight},
    {QT_TRANSLATE_NOOP("SettingsWidget", "redo"), RedoDrawing},
    {QT_TRANSLATE_NOOP("SettingsWidget", "redo left"), RedoDrawingLeft},
    {QT_TRANSLATE_NOOP("SettingsWidget", "redo right"), RedoDrawingRight},
    {QT_TRANSLATE_NOOP("SettingsWidget", "clear"), ClearDrawing},
    {QT_TRANSLATE_NOOP("SettingsWidget", "clear left"), ClearDrawingLeft},
    {QT_TRANSLATE_NOOP("SettingsWidget", "clear right"), ClearDrawingRight},
    {QT_TRANSLATE_NOOP("SettingsWidget", "scroll down"), ScrollDown},
    {QT_TRANSLATE_NOOP("SettingsWidget", "scroll up"), ScrollUp},
    {QT_TRANSLATE_NOOP("SettingsWidget", "scroll home"), ScrollNormal},
    {QT_TRANSLATE_NOOP("SettingsWidget", "scroll normal"), ScrollNormal},
    {QT_TRANSLATE_NOOP("SettingsWidget", "scroll reset"), ScrollNormal},
    {QT_TRANSLATE_NOOP("SettingsWidget", "zoom in"), ZoomIn},
    {QT_TRANSLATE_NOOP("SettingsWidget", "zoom out"), ZoomOut},
    {QT_TRANSLATE_NOOP("SettingsWidget", "zoom reset"), ZoomReset},
    {QT_TRANSLATE_NOOP("SettingsWidget", "save"), SaveDrawings},
    {QT_TRANSLATE_NOOP("SettingsWidget", "save as"), SaveDrawingsAs},
    {QT_TRANSLATE_NOOP("SettingsWidget", "open"), LoadDrawings},
    {QT_TRANSLATE_NOOP("SettingsWidget", "open unsafe"), LoadDrawingsNoClear},
    // Modify drawn items
    {QT_TRANSLATE_NOOP("SettingsWidget", "copy"), CopyClipboard},
    {QT_TRANSLATE_NOOP("SettingsWidget", "cut"), CutClipboard},
    {QT_TRANSLATE_NOOP("SettingsWidget", "paste"), PasteClipboard},
    {QT_TRANSLATE_NOOP("SettingsWidget", "to foreground"),
     SelectionToForeground},
    {QT_TRANSLATE_NOOP("SettingsWidget", "to background"),
     SelectionToBackground},
    {QT_TRANSLATE_NOOP("SettingsWidget", "delete"), RemoveSelectedItems},
    {QT_TRANSLATE_NOOP("SettingsWidget", "select all"), SelectAll},
    {QT_TRANSLATE_NOOP("SettingsWidget", "clear selection"), ClearSelection},
    // Timer
    {QT_TRANSLATE_NOOP("SettingsWidget", "start timer"), StartTimer},
    {QT_TRANSLATE_NOOP("SettingsWidget", "stop timer"), StopTimer},
    {QT_TRANSLATE_NOOP("SettingsWidget", "toggle timer"), StartStopTimer},
    {QT_TRANSLATE_NOOP("SettingsWidget", "reset timer"), ResetTimePassed},
    // Media
    {QT_TRANSLATE_NOOP("SettingsWidget", "play media"), PlayMedia},
    {QT_TRANSLATE_NOOP("SettingsWidget", "stop media"), PauseMedia},
    {QT_TRANSLATE_NOOP("SettingsWidget", "pause media"), PlayPauseMedia},
    {QT_TRANSLATE_NOOP("SettingsWidget", "mute"), Mute},
    {QT_TRANSLATE_NOOP("SettingsWidget", "unmute"), Unmute},
    // Other actions
    {QT_TRANSLATE_NOOP("SettingsWidget", "insert slide"), InsertSlide},
    {QT_TRANSLATE_NOOP("SettingsWidget", "remove slide"), RemoveSlide},
    {QT_TRANSLATE_NOOP("SettingsWidget", "restore slide"), RestoreSlide},
    {QT_TRANSLATE_NOOP("SettingsWidget", "reload"), ReloadFiles},
    {QT_TRANSLATE_NOOP("SettingsWidget", "fullscreen"), FullScreen},
    {QT_TRANSLATE_NOOP("SettingsWidget", "quit"), Quit},
    {QT_TRANSLATE_NOOP("SettingsWidget", "quit unsafe"), QuitNoConfirmation},
};

/// Convert string (from configuration file) to gesture
Gesture string_to_gesture(const QString &string) noexcept;

/// Map human readable string to overlay mode.
/// @see PdfMaster
static const QMap<QString, OverlayDrawingMode> string_to_overlay_mode{
    {"per page", PerPage},
    {"per label", PerLabel},
    {"cumulative", Cumulative},
};

static const QMap<PagePart, QString> page_part_names{
    {FullPage, "full page"},
    {LeftHalf, "left half"},
    {RightHalf, "right half"},
};

#ifdef QT_DEBUG
DebugFlag string_to_debug_flag(const QString &string) noexcept;
#endif  // QT_DEBUG

#endif  // NAMES_H
