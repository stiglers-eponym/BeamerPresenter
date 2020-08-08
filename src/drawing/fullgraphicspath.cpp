#include "fullgraphicspath.h"

FullGraphicsPath::FullGraphicsPath(const QPointF &pos, const float pressure)
{
    data.append({pos, pressure});
    top = pos.y() - pen.widthF();
    bottom = pos.y() + pen.widthF();
    left = pos.x() - pen.widthF();
    right = pos.x() + pen.widthF();
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
    top = other->data[first].point.y();
    bottom = top;
    left = other->data[first].point.x();
    right = left;
    for (int i=0; i<length; i++)
    {
        data[i] = other->data[i+first];
        if ( data[i].point.x() < left )
            left = data[i].point.x();
        else if ( data[i].point.x() > right )
            right = data[i].point.x();
        if ( data[i].point.y() < top )
            top = data[i].point.y();
        else if ( data[i].point.y() > bottom )
            bottom = data[i].point.y();
    }
    left -= pen.widthF();
    right += pen.widthF();
    top -= pen.widthF();
    bottom += pen.widthF();
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
    // Only for debugging
    //painter->setPen(QPen(QBrush(Qt::black), 0.5));
    //painter->drawRect(boundingRect());
}

void FullGraphicsPath::addPoint(const QPointF &point, const float pressure)
{
    data.append({point, pressure});
    bool change = false;
    if ( point.x() < left + pen.widthF() )
    {
        left = point.x() - pen.widthF();
        change = true;
    }
    else if ( point.x() + pen.widthF() > right )
    {
        right = point.x() + pen.widthF();
        change = true;
    }
    if ( point.y() < top + pen.widthF() )
    {
        top = point.y() - pen.widthF();
        change = true;
    }
    else if ( point.y() + pen.widthF() > bottom )
    {
        bottom = point.y() + pen.widthF();
        change = true;
    }
    if (change)
        prepareGeometryChange();
}

QList<AbstractGraphicsPath*> FullGraphicsPath::splitErase(const QPointF &pos, const qreal size) const
{
    if (!boundingRect().marginsAdded(QMarginsF(size, size, size, size)).contains(pos))
        // If returned list contains only a nullptr, this is interpreted as "no
        // changes".
        return {nullptr};

    QList<AbstractGraphicsPath*> list;
    const qreal sizesq = size*size;
    int first = 0, last = 0;
    while (last < data.length())
    {
        const QPointF diff = data[last++].point - pos;
        if (QPointF::dotProduct(diff, diff) < sizesq)
        {
            if (last > first + 2)
                list.append(new FullGraphicsPath(this, first, last-1));
            first = last;
        }
    }
    // If first == 0, then the path has not changed.
    if (first == 0)
        return {nullptr};
    if (last > first + 2)
        list.append(new FullGraphicsPath(this, first, last));
    return list;
}
