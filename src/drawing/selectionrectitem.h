// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SELECTIONRECTITEM_H
#define SELECTIONRECTITEM_H

#include <QGraphicsItem>

class SelectionRectItem : public QGraphicsItem
{
    QRectF _rect;
public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 10};

    /// Trivial constructor.
    SelectionRectItem(QGraphicsItem *parent = NULL) : QGraphicsItem(parent)
    {setZValue(1e2);}

    /// @return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Paint this on given painter.
    /// @param painter paint to this painter.
    /// @param option currently ignored.
    /// @param widget currently ignored.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// reset transform and set rect (in scene coordinates)
    void setRect(const QRectF &rect) noexcept;

    /// return (possibly rotated) rectangle in scene coordinates
    QPolygonF sceneRect() const noexcept
    {return mapToScene(_rect);}

    /// return rectangle center in scene coordinates
    QPointF sceneCenter() const noexcept
    {return mapToScene(_rect.center());}

    /// return center or rotation handle in scene coordinates
    QPointF sceneRotationHandle() const noexcept
    {return mapToScene(_rect.left()+_rect.width()/2, _rect.top()-10);}

    QPolygonF scaleHandles() const noexcept;

    virtual QRectF boundingRect() const noexcept override
    {return _rect.marginsAdded(QMarginsF(4,14,4,4));}
};

#endif // SELECTIONRECTITEM_H
