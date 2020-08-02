#include "fullgraphicspath.h"

FullGraphicsPath::FullGraphicsPath()
{

}

void FullGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (data.isEmpty())
        return;
    //painter->setOpacity(opacity());
    auto it = data.cbegin();
    while (++it != data.cend())
    {
        pen.setWidthF(it->pressure);
        painter->setPen(pen);
        painter->drawLine((it-1)->point, it->point);
    }
}

void FullGraphicsPath::addPoint(const QPointF &point, const float pressure)
{
    data.append({point, pressure});
    bool change = false;
    if ( point.x() < left + pressure )
    {
        left = point.x() - pressure;
        change = true;
    }
    else if ( point.x() + pressure > right )
    {
        right = point.x() + pressure;
        change = true;
    }
    if ( point.y() < top + pressure )
    {
        top = point.y() - pressure;
        change = true;
    }
    else if ( point.y() + pressure > bottom )
    {
        bottom = point.y() + pressure;
        change = true;
    }
    if (change)
        prepareGeometryChange();
}
