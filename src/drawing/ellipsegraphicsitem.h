// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ELLIPSEGRAPHICSITEM_H
#define ELLIPSEGRAPHICSITEM_H

#include <QPointF>
#include <QGraphicsEllipseItem>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/drawing/drawtool.h"

class QWidget;
class QPainter;
class BasicGraphicsPath;
class QStyleOptionGraphicsItem;

/**
 * @brief EllipseGraphicsItem: QGraphicsEllipseItem for flexible drawing of an ellipse
 *
 * This extends QGraphicsEllipseItem by checking that the rect is always valid.
 * It also contains a DrawTool and can be converted to a BasicGraphicsPath.
 */
class EllipseGraphicsItem : public QGraphicsEllipseItem
{
    enum {
        OriginRight = 0x1,
        OriginBottom = 0x2,
        TopLeft = 0x0,
        TopRight = OriginRight,
        BottomLeft = OriginBottom,
        BottomRight = OriginRight | OriginBottom,
    };
    /// DrawTool for this path.
    const DrawTool tool;
    /// Defines which corner of the rectangle is kept fixed.
    quint8 origin = TopLeft;

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + EllipsisGraphicsItemType };

    /// Constructor for initializing QGraphicsEllipseItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    EllipseGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = nullptr);

    /// Trivial destructor.
    ~EllipseGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the rectangle.
    /// Make sure that the underlying rect is always valid.
    void setSecondPoint(const QPointF &pos);

    /// Convert to a BasicGraphicsPath for simpler erasing.
    BasicGraphicsPath *toPath() const;

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override
    {
        painter->setCompositionMode(tool.compositionMode());
        QGraphicsEllipseItem::paint(painter, option, widget);
    }
};

#endif // ELLIPSEGRAPHICSITEM_H
