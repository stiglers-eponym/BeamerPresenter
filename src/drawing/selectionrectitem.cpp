// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QPen>
#include "src/drawing/selectionrectitem.h"
#include "src/preferences.h"

void SelectionRectItem::setRect(const QRectF &rect) noexcept
{
    resetTransform();
    setPos(rect.center());
    _rect = rect;
    _rect.translate(-pos());
}

void SelectionRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (_rect.isEmpty())
        return;
    // TODO: better control handles
    painter->setRenderHint(QPainter::Antialiasing);
    QPen pen = preferences()->selection_rect_pen;
    painter->setPen(pen);
    painter->setBrush(preferences()->selection_rect_brush);
    painter->drawRect(_rect);

    const qreal handle_size = preferences()->selection_rect_handle_size;
    QRectF tmprect(-handle_size/2, -handle_size/2, handle_size, handle_size);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    tmprect.translate(_rect.topLeft());
    painter->drawRect(tmprect);
    tmprect.translate(0, _rect.height()/2);
    painter->drawRect(tmprect);
    tmprect.translate(0, _rect.height()/2);
    painter->drawRect(tmprect);
    tmprect.translate(_rect.width()/2, 0);
    painter->drawRect(tmprect);
    tmprect.translate(_rect.width()/2, 0);
    painter->drawRect(tmprect);
    tmprect.translate(0, -_rect.height()/2);
    painter->drawRect(tmprect);
    tmprect.translate(0, -_rect.height()/2);
    painter->drawRect(tmprect);
    tmprect.translate(-_rect.width()/2, 0);
    painter->drawRect(tmprect);
    tmprect.translate(0, -1.5*handle_size);
    painter->drawEllipse(tmprect);
    tmprect.translate(2*handle_size, 0);
    painter->setBrush(QColor(255, 255, 255, 196));
    pen.setColor(Qt::red);
    painter->setPen(pen);
    painter->drawRect(tmprect);
    painter->drawLine(tmprect.bottomLeft(), tmprect.topRight());
    painter->drawLine(tmprect.bottomRight(), tmprect.topLeft());
}

QPolygonF SelectionRectItem::scaleHandles() const noexcept
{
    QPolygonF polygon(mapToScene(_rect));
    const QPointF center = _rect.center();
    polygon.append(mapToScene({center.x(), _rect.top()}));
    polygon.append(mapToScene({center.x(), _rect.bottom()}));
    polygon.append(mapToScene({_rect.left(), center.y()}));
    polygon.append(mapToScene({_rect.right(), center.y()}));
    return polygon;
}
