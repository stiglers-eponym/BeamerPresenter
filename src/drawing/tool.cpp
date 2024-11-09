// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QTabletEvent>
#include <QtConfig>
#if (QT_VERSION_MAJOR >= 6)
#include <QPointingDevice>
#endif
#include "src/drawing/dragtool.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/selectiontool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/tool.h"
#include "src/log.h"

std::shared_ptr<Tool> Tool::copy() const
{
  Tool* newtool;
  if (_tool & Tool::AnyDrawTool)
    newtool = new DrawTool(*static_cast<const DrawTool*>(this));
  else if (_tool & Tool::AnyPointingTool)
    newtool = new PointingTool(*static_cast<const PointingTool*>(this));
  else if (_tool & Tool::AnySelectionTool)
    newtool = new SelectionTool(*static_cast<const SelectionTool*>(this));
  else if (_tool == Tool::TextInputTool)
    newtool = new TextTool(*static_cast<const TextTool*>(this));
  else if (_tool == Tool::DragViewTool)
    newtool = new DragTool(*static_cast<const DragTool*>(this));
  else
    newtool = new Tool(*this);
  newtool->setDevice(_device);
  return std::shared_ptr<Tool>(newtool);
}

int tablet_event_to_input_device(const QTabletEvent* event)
{
  debug_verbose(DebugDrawing,
                event << event->pointerType() << event->buttons());
  if (event->pressure() <= 0 && event->type() != QEvent::TabletRelease)
    return Tool::TabletHover;
  switch (event->pointerType()) {
#if (QT_VERSION_MAJOR >= 6)
    case QPointingDevice::PointerType::Pen:
      if (event->buttons() & Qt::MiddleButton)
        return Tool::TabletMod;
      else
        return Tool::TabletPen;
    case QPointingDevice::PointerType::Eraser:
      return Tool::TabletEraser;
    case QPointingDevice::PointerType::Cursor:
      return Tool::TabletCursor;
    case QPointingDevice::PointerType::Generic:
      return Tool::MouseNoButton;
    case QPointingDevice::PointerType::Finger:
      return Tool::TouchInput;
#else
    case QTabletEvent::Pen:
      return Tool::TabletPen;
    case QTabletEvent::Eraser:
      return Tool::TabletEraser;
    case QTabletEvent::Cursor:
      return Tool::TabletCursor;
    case QTabletEvent::UnknownPointer:
      return Tool::TabletOther;
#endif
    default:
      return Tool::TabletOther;
  }
}

const char* tool_to_description(const Tool::BasicTool tool) noexcept
{
  switch (tool) {
    case Tool::NoTool:
      return QT_TRANSLATE_NOOP(
          "Tool", "no tool: follow links and control audio/video content");
    case Tool::Pen:
      return QT_TRANSLATE_NOOP(
          "Tool",
          "pen with variable width if the input device supports "
          "variable pressure");
    case Tool::FixedWidthPen:
      return QT_TRANSLATE_NOOP(
          "Tool",
          "pen with fixed width (independent of input device pressure)");
    case Tool::Eraser:
      return QT_TRANSLATE_NOOP("Tool", "eraser: deletes drawings");
    case Tool::Highlighter:
      return QT_TRANSLATE_NOOP(
          "Tool",
          "highlighter: fixed width drawing which only darkens colors "
          "(full color on white background, invisible on black background)");
    case Tool::Pointer:
      return QT_TRANSLATE_NOOP("Tool", "pointer");
    case Tool::Torch:
      return QT_TRANSLATE_NOOP(
          "Tool",
          "torch: darken the slide leaving only a disk unchanged "
          "to focus attention on this area");
    case Tool::Magnifier:
      return QT_TRANSLATE_NOOP("Tool", "enlarge part of the slide");
    case Tool::TextInputTool:
      return QT_TRANSLATE_NOOP("Tool", "add or edit text on slide");
    case Tool::BasicSelectionTool:
      return QT_TRANSLATE_NOOP("Tool", "select objects by clicking on them");
    case Tool::RectSelectionTool:
      return QT_TRANSLATE_NOOP("Tool", "select objects in a rectangle");
    case Tool::FreehandSelectionTool:
      return QT_TRANSLATE_NOOP("Tool", "select objects inside a drawn shape");
    case Tool::DragViewTool:
      return QT_TRANSLATE_NOOP("Tool",
                               "drag/move the view. When active, double-click "
                               "and touch gestures can be used to zoom in.");
    default:
      return "";
  };
}

const QMap<QString, Tool::BasicTool> get_string_to_tool() noexcept
{
  /// convert string (from configuration files or saved file) to tool
  static const QMap<QString, Tool::BasicTool> string_to_tool{
      {QT_TRANSLATE_NOOP("Tool", "none"), Tool::NoTool},
      {QT_TRANSLATE_NOOP("Tool", "pen"), Tool::Pen},
      {QT_TRANSLATE_NOOP("Tool", "fixed width pen"), Tool::FixedWidthPen},
      {QT_TRANSLATE_NOOP("Tool", "eraser"), Tool::Eraser},
      {QT_TRANSLATE_NOOP("Tool", "highlighter"), Tool::Highlighter},
      {QT_TRANSLATE_NOOP("Tool", "pointer"), Tool::Pointer},
      {QT_TRANSLATE_NOOP("Tool", "torch"), Tool::Torch},
      {QT_TRANSLATE_NOOP("Tool", "magnifier"), Tool::Magnifier},
      {QT_TRANSLATE_NOOP("Tool", "text"), Tool::TextInputTool},
      {QT_TRANSLATE_NOOP("Tool", "drag view"), Tool::DragViewTool},
      {QT_TRANSLATE_NOOP("Tool", "click select"), Tool::BasicSelectionTool},
      {QT_TRANSLATE_NOOP("Tool", "rectangle select"), Tool::RectSelectionTool},
      {QT_TRANSLATE_NOOP("Tool", "freehand select"),
       Tool::FreehandSelectionTool},
  };
  return string_to_tool;
}

const QMap<std::string, int>& get_string_to_input_device() noexcept
{
  /// convert string (from configuration file) to InputDevice
  static const QMap<std::string, int> string_to_input_device{
      {QT_TRANSLATE_NOOP("Tool", "touch"), Tool::TouchInput},
      {QT_TRANSLATE_NOOP("Tool", "tablet pen"), Tool::TabletPen},
      {QT_TRANSLATE_NOOP("Tool", "tablet mod"), Tool::TabletMod},
      {QT_TRANSLATE_NOOP("Tool", "tablet"),
       Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther |
           Tool::TabletMod},
      {QT_TRANSLATE_NOOP("Tool", "tablet eraser"), Tool::TabletEraser},
      {QT_TRANSLATE_NOOP("Tool", "tablet hover"), Tool::TabletHover},
      {QT_TRANSLATE_NOOP("Tool", "tablet cursor"), Tool::TabletCursor},
      {QT_TRANSLATE_NOOP("Tool", "tablet mod"), Tool::TabletMod},
      {QT_TRANSLATE_NOOP("Tool", "tablet other"), Tool::TabletOther},
      {QT_TRANSLATE_NOOP("Tool", "tablet all"),
       Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther |
           Tool::TabletEraser | Tool::TabletMod},
      {QT_TRANSLATE_NOOP("Tool", "all"), Tool::AnyNormalDevice},
      {QT_TRANSLATE_NOOP("Tool", "all+"), Tool::AnyPointingDevice},
      {QT_TRANSLATE_NOOP("Tool", "all++"), Tool::AnyDevice},
      {QT_TRANSLATE_NOOP("Tool", "left button"), Tool::MouseLeftButton},
      {QT_TRANSLATE_NOOP("Tool", "mouse"), Tool::MouseLeftButton},
      {QT_TRANSLATE_NOOP("Tool", "right button"), Tool::MouseRightButton},
      {QT_TRANSLATE_NOOP("Tool", "middle button"), Tool::MouseMiddleButton},
      {QT_TRANSLATE_NOOP("Tool", "no button"), Tool::MouseNoButton},
      {QT_TRANSLATE_NOOP("Tool", "double-click"), Tool::MouseDoubleClick},
  };
  return string_to_input_device;
}
