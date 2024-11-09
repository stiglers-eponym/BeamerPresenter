// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/names.h"

#include <QObject>
#include <cstring>
#include <map>

GuiWidget string_to_widget_type(const QString &string) noexcept
{
  static const std::map<const std::string, GuiWidget> lookup_table = {
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
      {"tools", ToolWidgetType},
      {"settings", SettingsType},
      {"search", SearchType},
      {"clock", ClockType},
      {"analog clock", AnalogClockType},
      {"timer", TimerType},
      {"slide number", SlideNumberType},
      {"slide label", SlideLabelType},
  };
  const auto find = lookup_table.find(string.toLower().toStdString());
  if (find == lookup_table.end()) return InvalidType;
  return find->second;
}

Gesture string_to_gesture(const QString &string) noexcept
{
  static const std::map<const std::string, Gesture> lookup_table = {
      {"swipe up", SwipeUp},
      {"swipe down", SwipeDown},
      {"swipe left", SwipeLeft},
      {"swipe right", SwipeRight},
  };
  const auto find = lookup_table.find(string.toLower().toStdString());
  if (find == lookup_table.end()) return InvalidGesture;
  return find->second;
}

const QMap<QString, Action> &get_string_to_action() noexcept
{
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
  return string_to_action_map;
}

const QMap<QString, OverlayDrawingMode> &get_string_to_overlay_mode() noexcept
{
  static const QMap<QString, OverlayDrawingMode> string_to_overlay_mode{
      {"per page", OverlayDrawingMode::PerPage},
      {"per label", OverlayDrawingMode::PerLabel},
      {"cumulative", OverlayDrawingMode::Cumulative},
  };
  return string_to_overlay_mode;
}

const QMap<PagePart, QString> &get_page_part_names() noexcept
{
  static const QMap<PagePart, QString> page_part_names{
      {FullPage, "full page"},
      {LeftHalf, "left half"},
      {RightHalf, "right half"},
  };
  return page_part_names;
}

#ifdef QT_DEBUG
DebugFlag string_to_debug_flag(const QString &string) noexcept
{
  static const std::map<const std::string, DebugFlag> lookup_table = {
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
      {"debug threads", DebugThreads},
      {"debug calls", DebugFunctionCalls},
      {"debug all", DebugAll},
      {"debug verbose", DebugVerbose},
  };
  const auto find = lookup_table.find(string.toLower().toStdString());
  if (find == lookup_table.end()) return NoLog;
  return find->second;
}
#endif  // QT_DEBUG
