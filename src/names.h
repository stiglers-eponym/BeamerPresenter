#ifndef NAMES_H
#define NAMES_H

#include <QString>
#include <QMap>
#include <QBoxLayout>
#include <QTabletEvent>
#include <QTabWidget>
#include "src/enumerates.h"

/// Convert strings to GuiWidget
static const QMap<QString, GuiWidget> string_to_widget_type
{
    {"horizontal", HBoxWidgetType},
    {"horizontal boxes", HBoxWidgetType},
    {"vertical", VBoxWidgetType},
    {"vertical boxes", VBoxWidgetType},
    {"stacked", StackedWidgetType},
    {"tabbed", TabbedWidgetType},
    {"slide", SlideType},
    {"thumbnails", OverviewType},
    {"toc", TOCType},
    {"table of contents", TOCType},
    {"outline", TOCType},
    {"notes", NotesType},
    {"tool selector", ToolSelectorType},
    {"settings", SettingsType},
    {"clock", ClockType},
    {"timer", TimerType},
    {"slide number", SlideNumberType},
    {"slide label", SlideLabelType},
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
    {"save", SaveDrawings},
    {"save as", SaveDrawingsAs},
    {"open", LoadDrawings},
    {"open unsafe", LoadDrawingsNoClear},
    {"undo", UndoDrawing},
    {"undo left", UndoDrawingLeft},
    {"undo right", UndoDrawingRight},
    {"redo", RedoDrawing},
    {"redo left", RedoDrawingLeft},
    {"redo right", RedoDrawingRight},
    {"clear", ClearDrawing},
    {"clear left", ClearDrawingLeft},
    {"clear right", ClearDrawingRight},
    {"start timer", StartTimer},
    {"stop timer", StopTimer},
    {"toggle timer", StartStopTimer},
    {"reset timer", ResetTimePassed},
    {"scroll up", ScrollUp},
    {"scroll down", ScrollDown},
    {"scroll normal", ScrollNormal},
    {"play media", PlayMedia},
    {"stop media", PauseMedia},
    {"pause media", PlayPauseMedia},
    {"full screen", FullScreen},
    {"fullscreen", FullScreen},
    {"quit", Quit},
    {"quit unsafe", QuitNoConfirmation},
};

static const QMap<QString, QBoxLayout::Direction> string_to_layout_direction
{
    {"horizontal", QBoxLayout::LeftToRight},
    {"vertical", QBoxLayout::TopToBottom},
};

static const QMap<QString, QTabWidget::TabPosition> string_to_tab_widget_orientation
{
    {"", QTabWidget::North},
    {"north", QTabWidget::North},
    {"south", QTabWidget::South},
    {"east",  QTabWidget::East},
    {"west",  QTabWidget::West},
};

static const QMap<QString, Qt::PenStyle> string_to_pen_style
{
    {"solid", Qt::SolidLine},
    {"dashed", Qt::DashLine},
    {"dotted", Qt::DotLine},
    {"dash-dotted", Qt::DashDotLine},
};

#endif // NAMES_H
