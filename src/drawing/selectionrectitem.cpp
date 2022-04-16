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
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(preferences()->selection_rect_pen);
    painter->setBrush(preferences()->selection_rect_brush);
    painter->drawRect(_rect);
    QRectF tmprect(-3.5,-3.5,7,7);
    painter->setPen(QPen(Qt::gray, 1));
    tmprect.translate(_rect.bottomLeft());
    painter->drawRect(tmprect);
    tmprect.translate(_rect.width(), 0);
    painter->drawRect(tmprect);
    tmprect.translate(0, -_rect.height());
    painter->drawRect(tmprect);
    tmprect.translate(-_rect.width(), 0);
    painter->drawRect(tmprect);
    tmprect.translate(_rect.width()/2, -10);
    painter->drawEllipse(tmprect);
}
