#include "basicgraphicspath.h"

BasicGraphicsPath::BasicGraphicsPath(const QPointF &pos)
{
    // Initialize bounding rect.
    top = pos.y() - pen.widthF();
    bottom = pos.y() + pen.widthF();
    left = pos.x() - pen.widthF();
    right = pos.x() + pen.widthF();
    // Add first data point.
    data.append(pos);
}

BasicGraphicsPath::BasicGraphicsPath(const BasicGraphicsPath * const other, int first, int last) :
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
    data = QVector<QPointF>(length);
    // Initialize bounding rect.
    top = other->data[first].y();
    bottom = top;
    left = other->data[first].x();
    right = left;
    // Copy data points from other and update bounding rect.
    for (int i=0; i<length; i++)
    {
        data[i] = other->data[i+first];
        if ( data[i].x() < left )
            left = data[i].x();
        else if ( data[i].x() > right )
            right = data[i].x();
        if ( data[i].y() < top )
            top = data[i].y();
        else if ( data[i].y() > bottom )
            bottom = data[i].y();
    }
    // Add finite stroke width to bounding rect.
    left -= pen.widthF();
    right += pen.widthF();
    top -= pen.widthF();
    bottom += pen.widthF();
}

void BasicGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (data.isEmpty())
        return;
    //painter->setOpacity(opacity());
    auto it = data.cbegin();
    while (++it != data.cend())
    {
        painter->drawLine(*(it-1), *it);
    }
    // Only for debugging
    //painter->setPen(QPen(QBrush(Qt::black), 0.5));
    //painter->drawRect(boundingRect());
}

void BasicGraphicsPath::addPoint(const QPointF &point)
{
    data.append(point);
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

QList<AbstractGraphicsPath*> BasicGraphicsPath::splitErase(const QPointF &pos, const qreal size) const
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
        const QPointF diff = data[last++] - pos;
        if (QPointF::dotProduct(diff, diff) < sizesq)
        {
            if (last > first + 2)
                list.append(new BasicGraphicsPath(this, first, last-1));
            first = last;
        }
    }
    // If first == 0, then the path has not changed.
    if (first == 0)
        return {nullptr};
    if (last > first + 2)
        list.append(new BasicGraphicsPath(this, first, last));
    return list;
}
