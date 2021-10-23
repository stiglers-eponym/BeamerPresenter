#ifndef TOOL_H
#define TOOL_H

#include "src/enumerates.h"
#include <QTabletEvent>

/**
 * @class Tool
 * @brief Basis class for tools on slides
 *
 * This container class is inherited by DrawTool, PointingTool and TextTool.
 * It contains a type (BasicTool) and device (InputDevice).
 */
class Tool
{
public:
    /// Tools for drawing and highlighting.
    /// The first 5 bits are used to encode a tool number.
    /// The following 3 bits are used as flags for classes of tools.
    enum BasicTool
    {
        // Invalid tool
        InvalidTool = 0,
        // General tools, class Tool
        TextInputTool = 1,
        // No tool
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
        Pointer = 0   | AnyPointingTool,
        Torch = 1     | AnyPointingTool,
        Magnifier = 2 | AnyPointingTool,
        Eraser = 3    | AnyPointingTool,

        // selection tools, class Tool
        RectSelectionTool = 0 | AnySelectionTool,
        FlexSelectionTool = 1 | AnySelectionTool,
    };

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
        AnyDevice = 0x0fff,
        AnyPointingDevice = AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
        AnyNormalDevice = AnyPointingDevice ^ (TabletHover | MouseNoButton),
        PressureSensitiveDevices = TabletPen | TabletEraser | TabletCursor | TabletOther,
    };

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

    Tool(const Tool &other) noexcept :
        _tool(other._tool), _device(other._device) {}

    /// Trivial virtual destructor.
    virtual ~Tool() {}

    virtual bool operator==(const Tool &other) const noexcept
    {return _tool == other._tool && _device == other._device;}

    BasicTool tool() const noexcept
    {return _tool;}

    int device() const noexcept
    {return _device;}

    void setDevice(const int device) noexcept
    {_device = device;}
};

/// convert string (from configuration files or saved file) to tool
static const QMap<QString, Tool::BasicTool> string_to_tool
{
    {"no tool", Tool::NoTool},
    {"none", Tool::NoTool},
    {"pen", Tool::Pen},
    {"fixed width pen", Tool::FixedWidthPen},
    {"eraser", Tool::Eraser},
    {"highlighter", Tool::Highlighter},
    {"pointer", Tool::Pointer},
    {"torch", Tool::Torch},
    {"magnifier", Tool::Magnifier},
    {"text", Tool::TextInputTool},
};

/// tool tip description of tools
static const QMap<Tool::BasicTool, QString> tool_to_description
{
    {Tool::Pen, "pen with variable width if the input device supports variable pressure"},
    {Tool::FixedWidthPen, "pen with fixed width (independent of input device pressure)"},
    {Tool::Eraser, "eraser: deletes drawings"},
    {Tool::Highlighter, "highlighter: fixed width drawing which only darkens colors (full color on white background, invisible on black background)"},
    {Tool::Pointer, "pointer"},
    {Tool::Torch, "torch: darken the slide leaving only a disk unchanged to focus attention on this area"},
    {Tool::Magnifier, "enlargen part of the slide"},
    {Tool::TextInputTool, "add or edit text on slide"},
};

/// convert QTabletEvent::PointerType to InputDevice
#if (QT_VERSION_MAJOR >= 6)
static const QMap<QPointingDevice::PointerType, Tool::InputDevice> tablet_device_to_input_device
{
    {QPointingDevice::PointerType::Pen, Tool::TabletPen},
    {QPointingDevice::PointerType::Eraser, Tool::TabletEraser},
    {QPointingDevice::PointerType::Cursor, Tool::TabletCursor},
    {QPointingDevice::PointerType::Unknown, Tool::TabletOther},
    {QPointingDevice::PointerType::Generic, Tool::MouseNoButton},
    {QPointingDevice::PointerType::Finger, Tool::TouchInput},
};
#else
static const QMap<QTabletEvent::PointerType, Tool::InputDevice> tablet_device_to_input_device
{
    {QTabletEvent::Pen, Tool::TabletPen},
    {QTabletEvent::Eraser, Tool::TabletEraser},
    {QTabletEvent::Cursor, Tool::TabletCursor},
    {QTabletEvent::UnknownPointer, Tool::TabletOther},
};
#endif

/// convert string (from configuration file) to InputDevice
static const QMap<QString, int> string_to_input_device
{
    {"touch", Tool::TouchInput},
    {"tablet pen", Tool::TabletPen},
    {"tablet", Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther},
    {"tablet eraser", Tool::TabletEraser},
    {"tablet hover", Tool::TabletHover},
    {"tablet all", Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther | Tool::TabletEraser},
    {"all", Tool::AnyNormalDevice},
    {"all+", Tool::AnyPointingDevice},
    {"all++", Tool::AnyDevice},
    {"left button", Tool::MouseLeftButton},
    {"mouse", Tool::MouseLeftButton},
    {"right button", Tool::MouseRightButton},
    {"middle button", Tool::MouseMiddleButton},
    {"no button", Tool::MouseNoButton},
};

#endif // TOOL_H
