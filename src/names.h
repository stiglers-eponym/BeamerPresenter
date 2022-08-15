// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef NAMES_H
#define NAMES_H

#include <QString>
#include <QMap>
#include <QBoxLayout>
#include <QTabWidget>
#include "src/config.h"
#include "src/enumerates.h"

/// Convert strings to GuiWidget
GuiWidget string_to_widget_type(const QString &string) noexcept;

/// Translate strings appearing in config to Actions
static const QMap<QString, Action> string_to_action_map
{
    // Nagivation actions
    {"update", Update},
    {"next", NextPage},
    {"previous", PreviousPage},
    {"next skipping overlays", NextSkippingOverlays},
    {"previous skipping overlays", PreviousSkippingOverlays},
    {"first", FirstPage},
    {"last", LastPage},
    // Drawing
    {"undo", UndoDrawing},
    {"undo left", UndoDrawingLeft},
    {"undo right", UndoDrawingRight},
    {"redo", RedoDrawing},
    {"redo left", RedoDrawingLeft},
    {"redo right", RedoDrawingRight},
    {"clear", ClearDrawing},
    {"clear left", ClearDrawingLeft},
    {"clear right", ClearDrawingRight},
    {"scroll down", ScrollDown},
    {"scroll up", ScrollUp},
    {"scroll home", ScrollNormal},
    {"scroll normal", ScrollNormal},
    {"scroll reset", ScrollNormal},
    {"save", SaveDrawings},
    {"save as", SaveDrawingsAs},
    {"open", LoadDrawings},
    {"open unsafe", LoadDrawingsNoClear},
    // Modify drawn items
    {"copy", CopyClipboard},
    {"cut", CutClipboard},
    {"paste", PasteClipboard},
    {"to foreground", SelectionToForeground},
    {"delete", RemoveSelectedItems},
    {"duplicate", DuplicateSelectedItems},
    {"select all", SelectAll},
    {"clear selection", ClearSelection},
    // Timer
    {"start timer", StartTimer},
    {"stop timer", StopTimer},
    {"toggle timer", StartStopTimer},
    {"reset timer", ResetTimePassed},
    // Media
    {"play media", PlayMedia},
    {"stop media", PauseMedia},
    {"pause media", PlayPauseMedia},
    {"mute", Mute},
    {"unmute", Unmute},
    // Other actions
    {"reload", ReloadFiles},
    {"full screen", FullScreen},
    {"fullscreen", FullScreen},
    {"quit", Quit},
    {"quit unsafe", QuitNoConfirmation},
};

/// Convert string (from configuration file) to gesture
Gesture string_to_gesture(const QString &string) noexcept;

/// Get description for action
const QString action_to_description(const Action action) noexcept;

static const QMap<QString, Qt::PenStyle> string_to_pen_style
{
    {"nopen", Qt::NoPen},
    {"solid", Qt::SolidLine},
    {"dash", Qt::DashLine},
    {"dot", Qt::DotLine},
    {"dashdot", Qt::DashDotLine},
    {"dashdotdot", Qt::DashDotDotLine},
};

static const QMap<QString, Qt::BrushStyle> string_to_brush_style
{
    {"NoBrush", Qt::NoBrush},
    {"SolidPattern", Qt::SolidPattern},
    {"Dense1Pattern", Qt::Dense1Pattern},
    {"Dense2Pattern", Qt::Dense2Pattern},
    {"Dense3Pattern", Qt::Dense3Pattern},
    {"Dense4Pattern", Qt::Dense4Pattern},
    {"Dense5Pattern", Qt::Dense5Pattern},
    {"Dense6Pattern", Qt::Dense6Pattern},
    {"Dense7Pattern", Qt::Dense7Pattern},
    {"HorPattern", Qt::HorPattern},
    {"VerPattern", Qt::VerPattern},
    {"CrossPattern", Qt::CrossPattern},
    {"BDiagPattern", Qt::BDiagPattern},
    {"FDiagPattern", Qt::FDiagPattern},
    {"DiagCrossPattern", Qt::DiagCrossPattern},
};

/// Map human readable string to overlay mode.
/// @see PdfMaster
static const QMap<QString, OverlayDrawingMode> string_to_overlay_mode
{
    {"per page", PerPage},
    {"per label", PerLabel},
    {"cumulative", Cumulative},
};

#ifdef QT_DEBUG
DebugFlags string_to_debug_flags(const QString &string) noexcept;
#endif // QT_DEBUG

#endif // NAMES_H
