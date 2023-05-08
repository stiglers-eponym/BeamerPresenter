// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOL_H
#define TOOL_H

#include <QColor>
#include <QMap>
#include <QString>
#include <QCoreApplication>
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
    enum BasicTool
    {
        // General tools, class Tool
        /// Invalid tool
        InvalidTool = 0,
        /// Text input tool
        TextInputTool = 1,
        /// No tool
        NoTool = 0x1f,

        // Classes of tools, defined by single bits
        AnyDrawTool = 1 << 5,
        AnyPointingTool = 1 << 6,
        AnySelectionTool = 1 << 7,

        // Draw tools: first 3 bits, class DrawTool
        Pen = 0           | AnyDrawTool,
        FixedWidthPen = 1 | AnyDrawTool,
        Highlighter = 2   | AnyDrawTool,

        // Highlighting tools: next 2 bits, class PointingTool
        Torch = 0     | AnyPointingTool,
        Eraser = 1    | AnyPointingTool,
        Magnifier = 2 | AnyPointingTool,
        Pointer = 3   | AnyPointingTool,

        // selection tools, class Tool
        BasicSelectionTool = 0 | AnySelectionTool,
        RectSelectionTool = 1 | AnySelectionTool,
        FreehandSelectionTool = 2 | AnySelectionTool,
    };
    Q_ENUM(BasicTool)

    /// Combinable flags defining input devices.
    /// Obtain Qt::MouseButton by taking InputDevice >> 1.
    enum InputDevice
    {
        NoDevice = 0,
        MouseNoButton = 1,
        MouseLeftButton = Qt::LeftButton << 1,
        MouseRightButton = Qt::RightButton << 1,
        MouseMiddleButton = Qt::MiddleButton << 1,
        TabletPen = 1 << 4,
        TabletEraser = 1 << 5,
        TabletCursor = 1 << 6,
        TabletOther = 1 << 7,
        TabletHover = 1 << 8,
        TouchInput = 1 << 9,
        TabletMod = 1 << 10,
        AnyDevice = 0x0fff,
        AnyPointingDevice = AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
        AnyNormalDevice = AnyPointingDevice ^ (TabletHover | MouseNoButton | TabletMod),
        PressureSensitiveDevices = TabletPen | TabletEraser | TabletCursor | TabletOther | TabletMod,
        AnyTabletDevice = TabletPen | TabletEraser | TabletCursor | TabletOther | TabletMod | TabletHover,
        AnyMouseDevice = MouseNoButton | MouseLeftButton | MouseRightButton | MouseMiddleButton,
        AnyActiveDevice = AnyDevice ^ (MouseNoButton | TabletHover),
    };
    Q_FLAG(InputDevice)

    /// Distinguish start, stop, update and cancel events.
    enum DeviceEventType
    {
        NoEvent = 1 << 12,
        StartEvent = 2 << 12,
        UpdateEvent = 3 << 12,
        StopEvent = 4 << 12,
        CancelEvent = 5 << 12,
        AnyEvent = 0xf000,
    };

protected:
    /// Type of the tool.
    const BasicTool _tool;

    /// Device is a InputDevice or a combination of these.
    int _device;

public:
    /// Trivial constructor.
    Tool(const BasicTool tool, const int device = AnyDevice) noexcept :
        _tool(tool), _device(device) {}

    /// Copy constructor.
    Tool(const Tool &other) noexcept :
        _tool(other._tool), _device(other._device) {}

    /// Trivial virtual destructor.
    virtual ~Tool() {}

    /// Comparison by basic tool and device.
    virtual bool operator==(const Tool &other) const noexcept
    {return _tool == other._tool && _device == other._device;}

    /// Return whether the tool itself should currently be visible.
    virtual bool visible() const noexcept
    {return true;}

    /// get function for _tool
    BasicTool tool() const noexcept
    {return _tool;}

    /// get function for _device
    int device() const noexcept
    {return _device;}

    /// set _device.
    void setDevice(const int device) noexcept
    {_device = device;}

    /// get color. Useless for this class, but very helpfull for child classes.
    virtual QColor color() const noexcept
    {return Qt::black;};

    /// set color. Useless for this class, but very helpfull for child classes.
    virtual void setColor(const QColor &color) noexcept {};

    /// create copy of this.
    Tool* copy() const;
};


/// convert string (from configuration files or saved file) to tool
static const QMap<QString, Tool::BasicTool> string_to_tool
{
    {QT_TRANSLATE_NOOP("Tool", "none"), Tool::NoTool},
    {QT_TRANSLATE_NOOP("Tool", "pen"), Tool::Pen},
    {QT_TRANSLATE_NOOP("Tool", "fixed width pen"), Tool::FixedWidthPen},
    {QT_TRANSLATE_NOOP("Tool", "eraser"), Tool::Eraser},
    {QT_TRANSLATE_NOOP("Tool", "highlighter"), Tool::Highlighter},
    {QT_TRANSLATE_NOOP("Tool", "pointer"), Tool::Pointer},
    {QT_TRANSLATE_NOOP("Tool", "torch"), Tool::Torch},
    {QT_TRANSLATE_NOOP("Tool", "magnifier"), Tool::Magnifier},
    {QT_TRANSLATE_NOOP("Tool", "text"), Tool::TextInputTool},
    {QT_TRANSLATE_NOOP("Tool", "click select"), Tool::BasicSelectionTool},
    {QT_TRANSLATE_NOOP("Tool", "rectangle select"), Tool::RectSelectionTool},
    {QT_TRANSLATE_NOOP("Tool", "freehand select"), Tool::FreehandSelectionTool},
};

/// tool tip description of tools
const char *tool_to_description(const Tool::BasicTool tool) noexcept;

/**
 * @brief Get input device from tablet event
 * @param event
 * @return device
 */
int tablet_event_to_input_device(const QTabletEvent* event);

/// convert string (from configuration file) to InputDevice
static const QMap<std::string, int> string_to_input_device
{
    {QT_TRANSLATE_NOOP("Tool", "touch"), Tool::TouchInput},
    {QT_TRANSLATE_NOOP("Tool", "tablet pen"), Tool::TabletPen},
    {QT_TRANSLATE_NOOP("Tool", "tablet mod"), Tool::TabletMod},
    {QT_TRANSLATE_NOOP("Tool", "tablet"), Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther | Tool::TabletMod},
    {QT_TRANSLATE_NOOP("Tool", "tablet eraser"), Tool::TabletEraser},
    {QT_TRANSLATE_NOOP("Tool", "tablet hover"), Tool::TabletHover},
    {QT_TRANSLATE_NOOP("Tool", "tablet cursor"), Tool::TabletCursor},
    {QT_TRANSLATE_NOOP("Tool", "tablet mod"), Tool::TabletMod},
    {QT_TRANSLATE_NOOP("Tool", "tablet other"), Tool::TabletOther},
    {QT_TRANSLATE_NOOP("Tool", "tablet all"), Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther | Tool::TabletEraser | Tool::TabletMod},
    {QT_TRANSLATE_NOOP("Tool", "all"), Tool::AnyNormalDevice},
    {QT_TRANSLATE_NOOP("Tool", "all+"), Tool::AnyPointingDevice},
    {QT_TRANSLATE_NOOP("Tool", "all++"), Tool::AnyDevice},
    {QT_TRANSLATE_NOOP("Tool", "left button"), Tool::MouseLeftButton},
    {QT_TRANSLATE_NOOP("Tool", "mouse"), Tool::MouseLeftButton},
    {QT_TRANSLATE_NOOP("Tool", "right button"), Tool::MouseRightButton},
    {QT_TRANSLATE_NOOP("Tool", "middle button"), Tool::MouseMiddleButton},
    {QT_TRANSLATE_NOOP("Tool", "no button"), Tool::MouseNoButton},
};

#endif // TOOL_H
