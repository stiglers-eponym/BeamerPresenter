// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOL_H
#define TOOL_H

#include <QColor>
#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <memory>

#include "src/config.h"

class QTabletEvent;

/**
 * @brief Basis class for all tools on slides.
 *
 * This container class is inherited by DrawTool, PointingTool and TextTool.
 * It contains a type (BasicTool) and device (InputDevice).
 */
class Tool
{
  Q_GADGET
  Q_DECLARE_TR_FUNCTIONS(Tool)

 public:
  /// Tools for drawing and highlighting.
  /// The first 5 bits are used to encode a tool number.
  /// The following 3 bits are used as flags for classes of tools.
  enum BasicTool {
    // General tools, class Tool
    /// Invalid tool
    InvalidTool = 0,
    /// Draw view
    DragViewTool = 1,
    /// Text input tool
    TextInputTool = 2,
    /// No tool
    NoTool = 0x1f,

    // Classes of tools, defined by single bits
    AnyDrawTool = 1 << 5,
    AnyPointingTool = 1 << 6,
    AnySelectionTool = 1 << 7,

    // Draw tools: first 3 bits, class DrawTool
    Pen = 0 | AnyDrawTool,
    FixedWidthPen = 1 | AnyDrawTool,
    Highlighter = 2 | AnyDrawTool,

    // Highlighting tools: next 2 bits, class PointingTool
    Torch = 0 | AnyPointingTool,
    Eraser = 1 | AnyPointingTool,
    Magnifier = 2 | AnyPointingTool,
    Pointer = 3 | AnyPointingTool,

    // selection tools, class Tool
    BasicSelectionTool = 0 | AnySelectionTool,
    RectSelectionTool = 1 | AnySelectionTool,
    FreehandSelectionTool = 2 | AnySelectionTool,
  };
  Q_ENUM(BasicTool)

  /// Combinable flags defining input devices.
  /// Obtain Qt::MouseButton by taking InputDevice >> 1.
  enum InputDevice {
    NoDevice = 0,
    MouseNoButton = 1,
    MouseLeftButton = 1 << 1,
    MouseRightButton = 1 << 2,
    MouseMiddleButton = 1 << 3,
    TabletPen = 1 << 4,
    TabletEraser = 1 << 5,
    TabletCursor = 1 << 6,
    TabletOther = 1 << 7,
    TabletHover = 1 << 8,
    TouchInput = 1 << 9,
    TabletMod = 1 << 10,
    MouseDoubleClick = 1 << 11,
    AnyDevice = 0x0fff,
    AnyPointingDevice =
        AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
    AnyNormalDevice =
        AnyPointingDevice ^ (TabletHover | MouseNoButton | TabletMod),
    PressureSensitiveDevices =
        TabletPen | TabletEraser | TabletCursor | TabletOther | TabletMod,
    AnyTabletDevice = TabletPen | TabletEraser | TabletCursor | TabletOther |
                      TabletMod | TabletHover,
    AnyMouseDevice =
        MouseNoButton | MouseLeftButton | MouseRightButton | MouseMiddleButton,
    AnyActiveDevice = AnyDevice ^ (MouseNoButton | TabletHover),
    // Distinguish singular, start, stop, update, and cancel events.
    SingularEvent = 1 << 12,  ///< Singular event, e.g. double-click
    StartEvent = 2 << 12,     ///< Start of an event, e.g. push mouse button
    UpdateEvent = 3 << 12,    ///< Update event, e.g. move mouse
    StopEvent = 4 << 12,      ///< End of event, e.g. release mouse button
    CancelEvent =
        5 << 12,        ///< Cancel event, e.g. if touch gesture is recognized
    AnyEvent = 0xf000,  ///< Any of the mentioned events types
  };
  Q_DECLARE_FLAGS(InputDevices, InputDevice)
  Q_FLAG(InputDevices)

  static InputDevices mouseButtonToDevice(Qt::MouseButtons buttons) noexcept
  {
    InputDevices devices;
    if (buttons.testFlag(Qt::LeftButton)) devices |= MouseLeftButton;
    if (buttons.testFlag(Qt::RightButton)) devices |= MouseRightButton;
    if (buttons.testFlag(Qt::MiddleButton)) devices |= MouseMiddleButton;
    if (devices == 0) devices = MouseNoButton;
    return devices;
  }

  /**
   * @brief Get input device from tablet event
   * @param event
   * @return device
   */
  static InputDevice tabletEventToInputDevice(const QTabletEvent *event);

 protected:
  /// Type of the tool.
  const BasicTool _tool;

  /// Device is a InputDevice or a combination of these.
  InputDevices _device;

 public:
  /// Trivial constructor.
  Tool(const BasicTool tool, const InputDevices device = AnyDevice) noexcept
      : _tool(tool), _device(device)
  {
  }

  /// Copy constructor.
  Tool(const Tool &other) noexcept : _tool(other._tool), _device(other._device)
  {
  }

  /// Trivial virtual destructor.
  virtual ~Tool() {}

  /// Comparison by basic tool and device.
  virtual bool operator==(const Tool &other) const noexcept
  {
    return _tool == other._tool && _device == other._device;
  }

  /// Return whether the tool itself should currently be visible.
  virtual bool visible() const noexcept { return true; }

  /// get function for _tool
  BasicTool tool() const noexcept { return _tool; }

  /// get function for _device
  InputDevices device() const noexcept { return _device; }

  /// set _device.
  void setDevice(const InputDevices device) noexcept { _device = device; }

  /// get color. Useless for this class, but very helpfull for child classes.
  virtual QColor color() const noexcept { return Qt::black; };

  /// set color. Useless for this class, but very helpfull for child classes.
  virtual void setColor(const QColor &color) noexcept {};

  /// create copy of this.
  std::shared_ptr<Tool> copy() const;

  /// tool tip description of tools
  virtual QString description() const noexcept;
};
Q_DECLARE_METATYPE(std::shared_ptr<Tool>);
Q_DECLARE_OPERATORS_FOR_FLAGS(Tool::InputDevices);

const QMap<QString, Tool::BasicTool> get_string_to_tool() noexcept;

const QMap<Tool::InputDevice, std::string> &get_device_to_string() noexcept;
const QMap<Tool::InputDevices, std::string> &get_devices_to_string() noexcept;

#endif  // TOOL_H
