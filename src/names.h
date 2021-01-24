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
    {"thumbnails", OverviewType},
    {"toc", TOCType},
    {"table of contents", TOCType},
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
    {"reset timer", ResetTimePassed},
    {"quit", Quit},
};

static const QMap<QString, BasicTool> string_to_tool
{
    {"no tool", NoTool},
    {"none", NoTool},
    {"pen", Pen},
    {"fixed width pen", FixedWidthPen},
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

static const QMap<QString, int> string_to_input_device
{
    {"touch", TouchInput},
    {"tablet pen", TabletPen},
    {"tablet", TabletPen | TabletCursor | TabletOther},
    {"tablet eraser", TabletEraser},
    {"tablet all", TabletPen | TabletCursor | TabletOther | TabletEraser},
    {"all", AnyDevice},
    {"mouse", MouseLeftButton},
    {"left button", MouseLeftButton},
    {"right button", MouseRightButton},
    {"middle button", MouseMiddleButton},
};

#endif // NAMES_H
