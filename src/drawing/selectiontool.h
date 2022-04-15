#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "src/drawing/tool.h"

class SelectionTool : public Tool
{
protected:
    QPointF reference_position;

public:
    SelectionTool(BasicTool base_tool, const int default_device) noexcept :
        Tool(base_tool, default_device) {}

    SelectionTool(const SelectionTool &other) noexcept :
        Tool(other), reference_position(other.reference_position) {}

    ~SelectionTool() {}

    void setPos(const QPointF &pos)
    {reference_position = pos;}

    const QPointF &pos()
    {return reference_position;}

    void clearPos()
    {reference_position = QPointF();}

    /// Set new reference position and return difference between new and old
    /// position.
    QPointF movePosition(const QPointF &new_position)
    {
        const QPointF diff = new_position - reference_position;
        reference_position = new_position;
        return diff;
    }
};

#endif // SELECTIONTOOL_H
