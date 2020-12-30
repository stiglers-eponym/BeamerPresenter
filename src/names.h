#ifndef NAMES_H
#define NAMES_H

#include <QString>
#include <QMap>
#include <QBoxLayout>
#include "src/enumerates.h"

/// Convert strings to GuiWidget
static const QMap<QString, GuiWidget> string_to_widget_type
{
    {"container", ContainerWidgetType},
    {"stacked", StackedWidgetType},
    {"slide", SlideType},
    {"overview", OverviewType},
    {"toc", TOCType},
    {"table of contents", TOCType},
    {"notes", NotesType},
    {"button", ButtonType},
    {"tool selector", ToolSelectorType},
    {"settings", SettingsType},
    {"clock", ClockType},
    {"timer", TimerType},
    {"slide number", SlideNumberType},
};

/// Translate strings appearing in config to Actions
static const QMap<QString, Action> string_to_action_map
{
    {"previous", PreviousPage},
    {"next", NextPage},
    {"first", FirstPage},
    {"last", LastPage},
    {"update", Update},
    {"next skipping overlays", NextSkippingOverlays},
    {"previous skipping overlays", PreviousSkippingOverlays},
    {"reload", ReloadFiles},
    {"undo", UndoDrawing},
    {"redo", RedoDrawing},
    {"clear", ClearDrawing},
    {"quit", Quit},
};

static const QMap<QString, QBoxLayout::Direction> string_to_layout_direction
{
    {"horizontal", QBoxLayout::LeftToRight},
    {"vertical", QBoxLayout::TopToBottom},
};

#endif // NAMES_H
