#ifndef NAMES_H
#define NAMES_H

#include <QString>
#include <QMap>
#include <QBoxLayout>
#include <QTabletEvent>
#include <QTabWidget>
#include "src/enumerates.h"

/// Convert strings to LogLevel components.
static const QMap<QString, LogLevel> string_to_log_level
{
    {"", NoLog},
    {"none", NoLog},
    {"slide changes", LogSlideChanges},
    {"debug rendering", DebugRendering},
    {"debug cache", DebugCache},
    {"debug drawing", DebugDrawing},
    {"debug media", DebugMedia},
    {"debug key-input", DebugKeyInput},
    {"debug settings", DebugSettings},
    {"debug transitions", DebugTransitions},
    {"debug page-change", DebugPageChange},
    {"debug layout", DebugLayout},
    {"debug widgets", DebugWidgets},
    {"debug all", DebugAll},
    {"debug verbose", DebugVerbose},
};

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
    {"load", LoadDrawings},
    {"load unsafe", LoadDrawingsNoClear},
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
    {"text", TextInputTool},
};

static const QMap<BasicTool, QString> xournal_tool_names
{
    {Pen, "pen"},
    {FixedWidthPen, "pen"},
    {Highlighter, "highlighter"},
    {TextInputTool, "text"},
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

static const QMap<QString, int> string_to_input_device
{
    {"touch", TouchInput},
    {"tablet pen", TabletPen},
    {"tablet", TabletPen | TabletCursor | TabletOther},
    {"tablet eraser", TabletEraser},
    {"tablet hover", TabletNoPressure},
    {"tablet all", TabletPen | TabletCursor | TabletOther | TabletEraser},
    {"all", AnyNormalDevice},
    {"all+", AnyPointingDevice},
    {"all++", AnyDevice},
    {"left button", MouseLeftButton},
    {"mouse", MouseLeftButton},
    {"right button", MouseRightButton},
    {"middle button", MouseMiddleButton},
    {"no button", MouseNoButton},
};

static const QMap<QString, OverlayDrawingMode> string_to_overlay_mode
{
    {"per page", PerPage},
    {"per label", PerLabel},
    {"cumulative", Cumulative},
};

#endif // NAMES_H
