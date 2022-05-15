#ifndef TOOL_H
#define TOOL_H

#include <QTabletEvent>
#include "src/enumerates.h"

/**
 * @brief Basis class for all tools on slides.
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
        Pointer = 0   | AnyPointingTool,
        Torch = 1     | AnyPointingTool,
        Magnifier = 2 | AnyPointingTool,
        Eraser = 3    | AnyPointingTool,

        // selection tools, class Tool
        BasicSelectionTool = 0 | AnySelectionTool,
        RectSelectionTool = 1 | AnySelectionTool,
        FreehandSelectionTool = 2 | AnySelectionTool,
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
        TabletMod = 1 << 10,
        AnyDevice = 0x0fff,
        AnyPointingDevice = AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
        AnyNormalDevice = AnyPointingDevice ^ (TabletHover | MouseNoButton | TabletMod),
        PressureSensitiveDevices = TabletPen | TabletEraser | TabletCursor | TabletOther | TabletMod,
    };

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
    {"click select", Tool::BasicSelectionTool},
    {"rectangle select", Tool::RectSelectionTool},
    {"freehand select", Tool::FreehandSelectionTool},
};

/// tool tip description of tools
static const QMap<Tool::BasicTool, const char*> tool_to_description
{
    {Tool::Pen, "pen with variable width if the input device supports variable pressure"},
    {Tool::FixedWidthPen, "pen with fixed width (independent of input device pressure)"},
    {Tool::Eraser, "eraser: deletes drawings"},
    {Tool::Highlighter, "highlighter: fixed width drawing which only darkens colors (full color on white background, invisible on black background)"},
    {Tool::Pointer, "pointer"},
    {Tool::Torch, "torch: darken the slide leaving only a disk unchanged to focus attention on this area"},
    {Tool::Magnifier, "enlargen part of the slide"},
    {Tool::TextInputTool, "add or edit text on slide"},
    {Tool::BasicSelectionTool, "Select objects by clicking on them"},
};

/**
 * @brief Get input device from tablet event
 * @param event
 * @return device
 */
int tablet_event_to_input_device(const QTabletEvent* event);

/// convert string (from configuration file) to InputDevice
static const QMap<QString, int> string_to_input_device
{
    {"touch", Tool::TouchInput},
    {"tablet pen", Tool::TabletPen},
    {"tablet", Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther | Tool::TabletMod},
    {"tablet eraser", Tool::TabletEraser},
    {"tablet hover", Tool::TabletHover},
    {"tablet all", Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther | Tool::TabletEraser | Tool::TabletMod},
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
