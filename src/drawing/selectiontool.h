#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "src/drawing/tool.h"

/**
 * @brief Tool for selection of QGraphicsItems
 */
class SelectionTool : public Tool
{
public:
    enum Operation {
        NoOperation,
        Move,
        Rotate,
        ResizeVertically,
        ResizeHorizontally,
        ResizeFlexible,
        ResizeFixed,
    };

protected:
    Operation operation;

    union {
        /** Position at which a selection items was grabbed, only active when
         * operation == Move.
         * This position (in scene coordinates) is used when moving objects.
         * reference_position is the input device position in scene coordinates
         * that was used for the previous step of moving a selection. */
        QPointF reference_position;
        qreal reference_angle;
    } properties {QPointF()};

public:
    /// trivial constructor, only initializes Tool
    SelectionTool(BasicTool base_tool, const int default_device) noexcept :
        Tool(base_tool, default_device) {}

    /// copy constructor
    SelectionTool(const SelectionTool &other) noexcept :
        Tool(other), properties(other.properties) {}

    /// trivial destructor
    ~SelectionTool() {}

    /// Set reference position.
    void setPos(const QPointF &pos)
    {properties.reference_position = pos;}

    /// return reference position.
    const QPointF &pos()
    {return properties.reference_position;}

    /** Set new reference position and return difference between new and old
     * position.
     * When an input device is used to move some objects, in each step of
     * moving objects this function should called with the new position of
     * the input device and the objects should be moved by the step returned
     * by this function.
     */
    QPointF movePosition(const QPointF &new_position)
    {
        const QPointF diff = new_position - properties.reference_position;
        properties.reference_position = new_position;
        return diff;
    }
};

#endif // SELECTIONTOOL_H
