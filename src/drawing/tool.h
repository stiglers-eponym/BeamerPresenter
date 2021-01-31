#ifndef TOOL_H
#define TOOL_H

#include "src/enumerates.h"

/// Container class for tools: draw tools and pointing tools.
class Tool
{
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

#endif // TOOL_H
