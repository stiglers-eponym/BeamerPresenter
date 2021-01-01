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
    {"tabbed", TabedWidgetType},
    {"slide", SlideType},
    {"overview", OverviewType},
    {"toc", TOCType},
    {"table of contents", TOCType},
    {"notes", NotesType},
    {"tool selector", ToolSelectorType},
    {"settings", SettingsType},
    {"clock", ClockType},
    {"timer", TimerType},
    {"slide number", SlideNumberType},
    {"slide label", SlideNumberType},
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

static const QMap<QString, BasicTool> string_to_tool
{
    {"no tool", NoTool},
    {"none", NoTool},
    {"pen", Pen},
    {"eraser", Eraser},
    {"highlighter", Highlighter},
    {"pointer", Pointer},
    {"torch", Torch},
    {"magnifier", Magnifier},
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

static const QMap<QTabletEvent::PointerType, InputDevice> tablet_device_to_input_device
{
    {QTabletEvent::Pen, TabletPen},
    {QTabletEvent::Eraser, TabletEraser},
    {QTabletEvent::Cursor, TabletCursor},
    {QTabletEvent::UnknownPointer, TabletOther},
};

static const QMap<Qt::MouseButtons, InputDevice> mouse_to_input_device
{
    {Qt::LeftButton, MouseLeftButton},
    {Qt::RightButton, MouseRightButton},
    {Qt::MiddleButton, MouseMiddleButton},
};

#endif // NAMES_H
