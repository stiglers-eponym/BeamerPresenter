#ifndef TOOL_H
#define TOOL_H

#include "src/enumerates.h"

class Tool
{
protected:
    const BasicTool _tool;

public:
    /// Trivial constructor.
    Tool(const BasicTool tool) noexcept : _tool(tool) {}

    BasicTool tool() const  noexcept
    {return _tool;}
};

#endif // TOOL_H
