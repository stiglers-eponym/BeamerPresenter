// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <map>
#include <cstring>
#include <QObject>
#include "src/names.h"

GuiWidget string_to_widget_type(const QString &string) noexcept
{
    static const std::map<const std::string, GuiWidget> lookup_table =
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
        {"search", SearchType},
        {"clock", ClockType},
        {"analog clock", AnalogClockType},
        {"timer", TimerType},
        {"slide number", SlideNumberType},
        {"slide label", SlideLabelType},
    };
    const auto find = lookup_table.find(string.toLower().toStdString());
    if (find == lookup_table.end())
        return InvalidType;
    return find->second;
}

Gesture string_to_gesture(const QString &string) noexcept
{
    static const std::map<const std::string, Gesture> lookup_table =
    {
        {"swipe up", SwipeUp},
        {"swipe down", SwipeDown},
        {"swipe left", SwipeLeft},
        {"swipe right", SwipeRight},
    };
    const auto find = lookup_table.find(string.toLower().toStdString());
    if (find == lookup_table.end())
        return InvalidGesture;
    return find->second;
}

const QString action_to_description(const Action action) noexcept
{
    switch (action)
    {
    case PreviousPage:
        return QObject::tr("go to previous page");
    case NextPage:
        return QObject::tr("go to next page");
    case FirstPage:
        return QObject::tr("go to first page");
    case LastPage:
        return QObject::tr("go to last page");
    case Update:
        return QObject::tr("update view (use if e.g. slide has bad resolution after resizing the window)");
    case NextSkippingOverlays:
        return QObject::tr("go to next page which has not the same page label as the current page");
    case PreviousSkippingOverlays:
        return QObject::tr("go to previous page which has not the same page label as the current page");
    case ReloadFiles:
        return QObject::tr("reload PDF file(s)");
    case SaveDrawings:
        return QObject::tr("save drawings, times and notes to file");
    case SaveDrawingsAs:
        return QObject::tr("ask where to save drawings, times and notes");
    case LoadDrawings:
        return QObject::tr("load drawings etc. from file");
    case LoadDrawingsNoClear:
        return QObject::tr("load drawings etc. from file without first clearing existing drawings");
    case UndoDrawing:
        return QObject::tr("undo last drawing/erasing step in the presentation on the current slide");
    case UndoDrawingLeft:
        return QObject::tr("undo last drawing/erasing step on the left half of a pdf document which is split into notes and a presentation");
    case UndoDrawingRight:
        return QObject::tr("undo last drawing/erasing step on the right half of a pdf document which is split into notes and a presentation");
    case RedoDrawing:
        return QObject::tr("redo undone drawing/erasing step in the presentation on the current slide");
    case RedoDrawingLeft:
        return QObject::tr("redo last drawing/erasing step on the left half of a pdf document which is split into notes and a presentation");
    case RedoDrawingRight:
        return QObject::tr("redo last drawing/erasing step on the right half of a pdf document which is split into notes and a presentation");
    case ClearDrawing:
        return QObject::tr("clear all drawings on the current presentation slide");
    case ClearDrawingLeft:
        return QObject::tr("clear all drawings of the current slide on the left half of a pdf document which is split into notes and a presentation");
    case ClearDrawingRight:
        return QObject::tr("clear all drawings of the current slide on the right half of a pdf document which is split into notes and a presentation");
    case StartTimer:
        return QObject::tr("start or resume presentation timer");
    case StopTimer:
        return QObject::tr("pause presentation timer");
    case StartStopTimer:
        return QObject::tr("pause or continue presentation timer");
    case ScrollDown:
        return QObject::tr("scroll down presentation view: adds extra space below the slide for drawing");
    case ScrollUp:
        return QObject::tr("scroll up presentation view");
    case ScrollNormal:
        return QObject::tr("reset view after scrolling to show full slide again");
    case PlayMedia:
        return QObject::tr("play all videos on the current slide");
    case PauseMedia:
        return QObject::tr("pause all videos on the current slide");
    case PlayPauseMedia:
        return QObject::tr("If any video is playing: pause all videos. Otherwise: start all videos.");
    case FullScreen:
        return QObject::tr("toggle fullscreen mode for currently active window");
    case Quit:
        return QObject::tr("quit and ask to save drawings if there are unsaved changes. Detection of unsaved chagnes is not reliable yet.");
    case QuitNoConfirmation:
        return QObject::tr("quit without asking to save drawings");
    default:
        return "";
    };
}


#ifdef QT_DEBUG
DebugFlags string_to_debug_flags(const QString &string) noexcept
{
    static const std::map<const std::string, DebugFlags> lookup_table =
    {
        {"debug rendering", DebugRendering},
        {"debug cache", DebugCache},
        {"debug drawing", DebugDrawing},
        {"debug media", DebugMedia},
        {"debug key-input", DebugKeyInput},
        {"debug other-input", DebugOtherInput},
        {"debug settings", DebugSettings},
        {"debug transitions", DebugTransitions},
        {"debug page-change", DebugPageChange},
        {"debug layout", DebugLayout},
        {"debug widgets", DebugWidgets},
        {"debug all", DebugAll},
        {"debug verbose", DebugVerbose},
    };
    const auto find = lookup_table.find(string.toLower().toStdString());
    if (find == lookup_table.end())
        return NoLog;
    return find->second;
}
#endif // QT_DEBUG
