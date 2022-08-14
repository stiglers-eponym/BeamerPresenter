// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPainter>
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
    painter->setPen(preferences()->selection_rect_pen);
    painter->setBrush(preferences()->selection_rect_brush);
    painter->drawRect(_rect);
    QRectF tmprect(-3.5,-3.5,7,7);
    painter->setPen(QPen(Qt::gray, 1));
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
    tmprect.translate(0, -10);
    painter->drawEllipse(tmprect);
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
