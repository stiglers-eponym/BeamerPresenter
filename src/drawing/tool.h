#ifndef TOOL_H
#define TOOL_H

#include "src/enumerates.h"
#include <QTabletEvent>

/// Container class for tools: draw tools and pointing tools.
class Tool
{
public:
    /// Tools for drawing and highlighting.
    enum BasicTool
    {
        // Invalid
        InvalidTool = 0,
        // Draw tools: first 4 bits, class DrawTool
        Pen = 1 << 0,
        FixedWidthPen = 2 << 0,
        Highlighter = 3 << 0,
        Eraser = 4 << 0,
        AnyDrawTool = 0x0f << 0,
        // Highlighting tools: next 4 bits, class PointingTool
        Pointer = 1 << 4,
        Torch = 2 << 4,
        Magnifier = 3 << 4,
        AnyPointingTool = 0x0f << 4,
        // Other tools, class Tool
        TextInputTool = 1 << 8,
        // No tool
        NoTool = 1 << 11,
    };

    /**
     * Obtain Qt::MouseButton by taking InputDevice >> 1.
     */
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
        TabletNoPressure = 1 << 8,
        TouchInput = 1 << 9,
        AnyDevice = 0xffff,
        AnyPointingDevice = AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
        AnyNormalDevice = AnyPointingDevice ^ (TabletNoPressure | MouseNoButton),
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

    BasicTool tool() const  noexcept
    {return _tool;}

    int device() const  noexcept
    {return _device;}

    void setDevice(const int device)  noexcept
    {_device = device;}
};


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

static const QMap<QTabletEvent::PointerType, Tool::InputDevice> tablet_device_to_input_device
{
    {QTabletEvent::Pen, Tool::TabletPen},
    {QTabletEvent::Eraser, Tool::TabletEraser},
    {QTabletEvent::Cursor, Tool::TabletCursor},
    {QTabletEvent::UnknownPointer, Tool::TabletOther},
};

static const QMap<QString, int> string_to_input_device
{
    {"touch", Tool::TouchInput},
    {"tablet pen", Tool::TabletPen},
    {"tablet", Tool::TabletPen | Tool::TabletCursor | Tool::TabletOther},
    {"tablet eraser", Tool::TabletEraser},
    {"tablet hover", Tool::TabletNoPressure},
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
