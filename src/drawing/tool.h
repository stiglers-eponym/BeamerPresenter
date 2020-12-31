#ifndef TOOL_H
#define TOOL_H

#include "src/enumerates.h"

/// Container class for tools: draw tools and pointing tools.
class Tool
{
protected:
    /// Type of the tool.
    const BasicTool _tool;

public:
    /// Trivial constructor.
    Tool(const BasicTool tool) noexcept : _tool(tool) {}

    BasicTool tool() const  noexcept
    {return _tool;}
};

#endif // TOOL_H
