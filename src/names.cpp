#include <map>
#include <cstring>
#include <stdexcept>
#include <QString>
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
        {"clock", ClockType},
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
    static const std::map<Action, const char*> lookup_table
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
    const auto find = lookup_table.find(action);
    if (find == lookup_table.end())
        return "";
    return QObject::tr(find->second);
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
