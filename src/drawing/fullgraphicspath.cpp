#include "fullgraphicspath.h"

FullGraphicsPath::FullGraphicsPath()
{

}

FullGraphicsPath::FullGraphicsPath(const FullGraphicsPath *other, int first, int last) :
    pen(other->pen)
{
    if (first < 0)
        first = 0;
    if (last > other->size())
        last = other->size();
    const int length = last - first;
    data = QVector<PointPressure>(length);
    for (int i=0; i<length; i++)
    {
        data[i] = other->data[i+first];
        if ( data[i].point.x() < left + data[i].pressure )
            left = data[i].point.x() - data[i].pressure;
        else if ( data[i].point.x() + data[i].pressure > right )
            right = data[i].point.x() + data[i].pressure;
        if ( data[i].point.y() < top + data[i].pressure )
            top = data[i].point.y() - data[i].pressure;
        else if ( data[i].point.y() + data[i].pressure > bottom )
            bottom = data[i].point.y() + data[i].pressure;
    }
    if (other->scene())
    {
        other->scene()->addItem(this);
        stackBefore(other);
    }
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

QList<AbstractGraphicsPath*> FullGraphicsPath::splitErase(const QPointF &pos, const qreal size) const
{
    QList<AbstractGraphicsPath*> list;
    if (contains(pos))
    {
        int first = 0, last = 0;
        while (last < data.length())
        {
            if ((data[last++].point - pos).manhattanLength() < size)
            {
                if (last > first + 2)
                    list.append(new FullGraphicsPath(this, first, last-1));
                first = last;
            }
        }
        if (first != 0 && last > first + 2)
            list.append(new FullGraphicsPath(this, first, last));
    }
    return list;
}
