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

static const QMap<Action, QString> action_to_description
{
    {PreviousPage, "go to previous page"},
    {NextPage, "go to next page"},
    {FirstPage, "go to first page"},
    {LastPage, "go to last page"},
    {Update, "update view (use if e.g. slide has bad resolution after resizing the window)"},
    {NextSkippingOverlays, "go to next page which has not the same page label as the current page"},
    {PreviousSkippingOverlays, "go to previous page which has not the same page label as the current page"},
    {ReloadFiles, "reload PDF file(s)."},
    {SaveDrawings, "save drawings, times and notes to file"},
    {SaveDrawingsAs, "ask where to save drawings, times and notes"},
    {LoadDrawings, "load drawings etc. from file"},
    {LoadDrawingsNoClear, "load drawings etc. from file without first clearing existing drawings"},
    {UndoDrawing, "undo last drawing/erasing step in the presentation on the current slide"},
    {UndoDrawingLeft, "undo last drawing/erasing step on the left half of a pdf document which is split into notes and a presentation"},
    {UndoDrawingRight, "undo last drawing/erasing step on the right half of a pdf document which is split into notes and a presentation"},
    {RedoDrawing, "redo undone drawing/erasing step in the presentation on the current slide"},
    {RedoDrawingLeft, "redo last drawing/erasing step on the left half of a pdf document which is split into notes and a presentation"},
    {RedoDrawingRight, "redo last drawing/erasing step on the right half of a pdf document which is split into notes and a presentation"},
    {ClearDrawing, "clear all drawings on the current presentation slide"},
    {ClearDrawingLeft, "clear all drawings of the current slide on the left half of a pdf document which is split into notes and a presentation"},
    {ClearDrawingRight, "clear all drawings of the current slide on the right half of a pdf document which is split into notes and a presentation"},
    {StartTimer, "start or resume presentation timer"},
    {StopTimer, "pause presentation timer"},
    {StartStopTimer, "pause or continue presentation timer"},
    {ScrollDown, "scroll down presentation view: adds extra space below the slide for drawing"},
    {ScrollUp, "scroll up presentation view"},
    {ScrollNormal, "reset view after scrolling to show full slide again"},
    {PlayMedia, "play all videos on the current slide"},
    {PauseMedia, "pause all videos on the current slide"},
    {PlayPauseMedia, "If any video is playing: pause all videos. Otherwise: start all videos."},
    {FullScreen, "toggle fullscreen mode for currently active window"},
    {Quit, "quit and ask to save drawings if there are unsaved changes. Detection of unsaved chagnes is not reliable yet."},
    {QuitNoConfirmation, "quit without asking to save drawings"},
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
