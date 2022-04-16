#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include <QGraphicsItem>
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
        Resize,
    };

protected:
    /// Type of transformation.
    Type _type = NoOperation;

    /// Properties needed to describe the transformation.
    union {
        /// 2 Points as used by most tools
        struct {
            QPointF start_pos;
            QPointF live_pos;
        } general;
        /// 2 Points as used for scaling
        struct {
            QPointF start_handle;
            QPointF live_handle;
            QPointF reference;
        } scale;
        /// Properties needed for rotation
        struct {
            QPointF rotation_center;
            qreal start_angle;
            qreal live_angle;
        } rotate;
    } properties {QPointF(), QPointF()};

    QHash<QGraphicsItem*, QTransform> initial_transforms;

public:
    /// trivial constructor, only initializes Tool
    SelectionTool(BasicTool base_tool, const int default_device) noexcept :
        Tool(base_tool, default_device) {}

    /// copy constructor
    SelectionTool(const SelectionTool &other) noexcept :
        Tool(other), properties(other.properties) {}

    /// trivial destructor
    ~SelectionTool() {}

    /// Set reference position and set _type to Move.
    void startMove(const QPointF &pos) noexcept;

    /// Set start position and set _type to Selection.
    void startRectSelection(const QPointF &pos) noexcept;

    /// type of operation.
    Type type() const noexcept
    {return _type;}

    /// reset type and items.
    void reset() noexcept;

    /// initialize initial_transformations.
    void initTransformations(const QList<QGraphicsItem*> &items) noexcept;

    const QHash<QGraphicsItem*, QTransform> &originalTransforms() const noexcept
    {return initial_transforms;}

    /// set _type.
    void setType(const Type type) noexcept
    {_type = type;}

    /// Set live position of rotation.
    void setLiveRotation(const QPointF &pos) noexcept;

    /// Set live position for resizing.
    void setLiveScaling(const QPointF &pos) noexcept;

    /// Set live position for moving.
    void setLiveMoving(const QPointF &pos) noexcept;

    /// Return fixed point (reference) of resize transformation.
    const QPointF &resizeReference() const noexcept
    {return properties.scale.reference;}

    /// return reference position.
    const QPointF &livePos() const noexcept
    {return properties.general.live_pos;}

    /// return reference position.
    const QPointF &startPos() const noexcept
    {return properties.general.start_pos;}

    /// Initialize rotation with reference point and rotation center.
    void startRotation(const QPointF &reference, const QPointF &center) noexcept;

    /// Initialize rotation with reference point and rotation center.
    void startScaling(const QPointF &handle, const QPointF &fixed) noexcept;

    /// angle of rotation (in degrees).
    qreal rotationAngle() const noexcept
    {return properties.rotate.live_angle - properties.rotate.start_angle;}

    /// return rotation center.
    const QPointF &rotationCenter() const noexcept
    {return properties.rotate.rotation_center;}

    /** Set new reference position and return difference between new and old
     * position.
     * When an input device is used to move some objects, in each step of
     * moving objects this function should called with the new position of
     * the input device and the objects should be moved by the step returned
     * by this function.
     */
    QPointF movePosition(const QPointF &new_position) noexcept;
};

#endif // SELECTIONTOOL_H
