// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef LINEGRAPHICSITEM_H
#define LINEGRAPHICSITEM_H

#include <QPointF>
#include <QLineF>
#include <QGraphicsLineItem>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/drawing/drawtool.h"

class QWidget;
class QPainter;
class BasicGraphicsPath;
class QStyleOptionGraphicsItem;

/**
 * @brief LineGraphicsItem: QGraphicsLineItem with a tool that can be converted to BasicGraphicsPath
 */
class LineGraphicsItem : public QGraphicsLineItem
{
    /// DrawTool for this path.
    const DrawTool tool;

public:
    /// QGraphicsItem type for this subclass
    enum { Type = UserType + LineGraphicsItemType };

    /// Constructor for initializing QGraphicsLineItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    LineGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = nullptr) :
        QGraphicsLineItem(QLineF(pos, pos), parent),
        tool(tool)
    {setPen(tool.pen());}

    /// Trivial destructor.
    ~LineGraphicsItem() noexcept {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the line.
    void setSecondPoint(const QPointF &pos) noexcept;

    /// Convert to a BasicGraphicsPath for simpler erasing.
    BasicGraphicsPath *toPath() const noexcept;

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif // LINEGRAPHICSITEM_H
