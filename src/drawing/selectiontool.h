#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "src/drawing/tool.h"

/**
 * @brief Tool for selection of QGraphicsItems
 */
class SelectionTool : public Tool
{
public:
    enum Type {
        NoOperation = 0,
        Select,
        Move,
        Rotate,
        ScaleTopLeft,
        ScaleTopRight,
        ScaleBottomLeft,
        ScaleBottomRight,
    };

protected:
    Type _type = NoOperation;

    union TransformProperties {
        /** Position at which a selection items was grabbed, only active when
         * _type == Move.
         * This position (in scene coordinates) is used when moving objects.
         * reference_position is the input device position in scene coordinates
         * that was used for the previous step of moving a selection. */
        QPointF position;
        qreal angle;
    };

    TransformProperties
        start_properties {QPointF()},
        live_properties {QPointF()};

public:
    /// trivial constructor, only initializes Tool
    SelectionTool(BasicTool base_tool, const int default_device) noexcept :
        Tool(base_tool, default_device) {}

    /// copy constructor
    SelectionTool(const SelectionTool &other) noexcept :
        Tool(other), start_properties(other.start_properties), live_properties(other.live_properties) {}

    /// trivial destructor
    ~SelectionTool() {}

    /// Set reference position and set _type to Move.
    void setPos(const QPointF &pos) noexcept;

    Type type() const noexcept
    {return _type;}

    /// return reference position.
    const QPointF &livePos() const noexcept
    {return live_properties.position;}

    /// return reference position.
    const QPointF &startPos() const noexcept
    {return start_properties.position;}

    /** Set new reference position and return difference between new and old
     * position.
     * When an input device is used to move some objects, in each step of
     * moving objects this function should called with the new position of
     * the input device and the objects should be moved by the step returned
     * by this function.
     */
    QPointF movePosition(const QPointF &new_position) noexcept;

    /// Transformation from start_properties to live_properties.
    QTransform transform() const;
};

#endif // SELECTIONTOOL_H
