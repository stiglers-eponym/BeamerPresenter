// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ARROWGRAPHICSITEM_H
#define ARROWGRAPHICSITEM_H

#include <QPointF>
#include <QGraphicsPathItem>
#include "src/config.h"
#include "src/drawing/drawtool.h"

class QWidget;
class QPainter;
class BasicGraphicsPath;
class QStyleOptionGraphicsItem;

/**
 * @brief ArrowGraphicsItem: Path shaped like an arrow
 *
 * Given a fixed start point and a flexible (movable) end point, this class
 * draws the arrow and can export the path as BasicGraphicsPath.
 */
class ArrowGraphicsItem : public QGraphicsPathItem
{
    /// DrawTool for this path.
    const DrawTool tool;
    /// Origin of the arrow
    const QPointF origin;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 9};

    /// Constructor for initializing QGraphicsLineItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    ArrowGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = nullptr) :
        QGraphicsPathItem(parent),
        origin(pos),
        tool(tool)
    {setPen(tool.pen());}

    /// Trivial destructor.
    ~ArrowGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the line.
    void setSecondPoint(const QPointF &pos);

    /// Convert to two BasicGraphicsPaths for simpler erasing.
    /// Two paths are needed because an arrow consists of two separate paths.
    QList<BasicGraphicsPath*> toPath() const;

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif // ARROWGRAPHICSITEM_H
