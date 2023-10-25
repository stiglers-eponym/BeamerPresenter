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

#ifdef QT_DEBUG
DebugFlags string_to_debug_flags(const QString &string) noexcept
{
  static const std::map<const std::string, DebugFlags> lookup_table = {
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
  if (find == lookup_table.end()) return NoLog;
  return find->second;
}
#endif  // QT_DEBUG
