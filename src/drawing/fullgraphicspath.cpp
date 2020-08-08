#include "fullgraphicspath.h"

FullGraphicsPath::FullGraphicsPath(const QPointF &pos, const float pressure)
{
    // Initialize bounding rect.
    top = pos.y() - pen.widthF();
    bottom = pos.y() + pen.widthF();
    left = pos.x() - pen.widthF();
    right = pos.x() + pen.widthF();
    // Add first data point.
    data.append({pos, pressure});
}

FullGraphicsPath::FullGraphicsPath(const FullGraphicsPath * const other, int first, int last) :
    AbstractGraphicsPath(other->pen)
{
    // Make sure that first and last are valid.
    if (first < 0)
        first = 0;
    if (last > other->size())
        last = other->size();
    const int length = last - first;
    if (length <= 0)
        // This should never happen.
        return;
    // Initialize data with the correct length.
    data = QVector<PointPressure>(length);
    // Initialize bounding rect.
    top = other->data[first].point.y();
    bottom = top;
    left = other->data[first].point.x();
    right = left;
    // Copy data points from other and update bounding rect.
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
    // Add finite stroke width to bounding rect.
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
