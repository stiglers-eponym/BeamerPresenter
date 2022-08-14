// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

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
        SelectRect,
        SelectPolygon,
        Move,
        Rotate,
        Resize,
    };

protected:
    /// Type of transformation.
    Type _type = NoOperation;

    /// Properties needed to describe the transformation.
    union {
        struct {
            /// start point as used by most tools, and always while selecting objects
            QPointF start_pos;
            QPointF current_pos;
        } general;
        /// Polygon representing selection boundary
        QPolygonF *polygon {nullptr};
        /// 2 Points as used for scaling
        struct {
            QPointF start_handle;
            QPointF reference;
        } scale;
        /// Properties needed for rotation
        struct {
            QPointF rotation_center;
            qreal start_angle;
        } rotate;
    } properties {QPointF()};

    /// Pointer to scene at which this tool is currently active. _scene is used
    /// by slide views to determine whether this tool should be drawn.
    const void *_scene {nullptr};

    /// Overwrite _type, delete polygon if necessary.
    void changeType(const Type newtype);

    QHash<QGraphicsItem*, QTransform> initial_transforms;

public:
    /// trivial constructor, only initializes Tool
    SelectionTool(BasicTool base_tool, const int default_device) noexcept :
        Tool(base_tool, default_device) {}

    /// copy constructor
    SelectionTool(const SelectionTool &other) noexcept :
        Tool(other), properties(other.properties) {}

    /// trivial destructor
    ~SelectionTool()
    {if (_type == SelectPolygon) delete properties.polygon;}

    bool visible() const noexcept
    {return _type == SelectRect || _type == SelectPolygon;}

    /// Set reference position and set _type to Move.
    void startMove(const QPointF &pos) noexcept;

    /// Set start position and set _type to SelectRect.
    void startRectSelection(const QPointF &pos, const void *scene) noexcept;

    /// Set _type to SelectPolygon and initialize polygon.
    void startFreehandSelection(const QPointF &pos, const void *scene) noexcept;

    /// type of operation.
    Type type() const noexcept
    {return _type;}

    /// scene on which this should be drawn.
    const void *scene() const noexcept
    {return _scene;}

    /// polygon representing selection boundary.
    const QPolygonF polygon() const noexcept;

    /// reset type and items.
    void reset() noexcept;

    /// initialize initial_transformations.
    void initTransformations(const QList<QGraphicsItem*> &items) noexcept;

    const QHash<QGraphicsItem*, QTransform> &originalTransforms() const noexcept
    {return initial_transforms;}

    /// set live update based on _type.
    void liveUpdate(const QPointF &pos) noexcept;

    /// set live position of rotation.
    void setLiveRotation(const QPointF &pos) noexcept;

    /// set live position for resizing.
    void setLiveScaling(const QPointF &pos) noexcept;

    /// return reference position.
    const QPointF &startPos() const noexcept
    {return properties.general.start_pos;}

    /// Initialize rotation with reference point and rotation center.
    void startRotation(const QPointF &reference, const QPointF &center) noexcept;

    /// Initialize rotation with reference point and rotation center.
    void startScaling(const QPointF &handle, const QPointF &fixed) noexcept;
};

#endif // SELECTIONTOOL_H
