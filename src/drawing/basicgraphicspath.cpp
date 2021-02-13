#include "basicgraphicspath.h"

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QPointF &pos) noexcept :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    top = pos.y() - tool.width();
    bottom = pos.y() + tool.width();
    left = pos.x() - tool.width();
    right = pos.x() + tool.width();
    // Add first data point.
    data.append(pos);
}

BasicGraphicsPath::BasicGraphicsPath(const BasicGraphicsPath * const other, int first, int last) :
    AbstractGraphicsPath(other->tool)
{
    // Make sure that first and last are valid.
    if (first < 0)
        first = 0;
    if (last > other->size() || last < 0)
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
    left -= tool.width();
    right += tool.width();
    top -= tool.width();
    bottom += tool.width();
}

void BasicGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (data.isEmpty())
        return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(tool.pen());
    painter->setCompositionMode(tool.compositionMode());
    painter->drawPolyline(data.constData(), data.size());

    // Only for debugging
    //painter->setPen(QPen(QBrush(Qt::black), 0.5));
    //painter->drawRect(boundingRect());
}

void BasicGraphicsPath::addPoint(const QPointF &point)
{
    data.append(point);
    bool change = false;
    if ( point.x() < left + tool.width() )
    {
        left = point.x() - tool.width();
        change = true;
    }
    else if ( point.x() + tool.width() > right )
    {
        right = point.x() + tool.width();
        change = true;
    }
    if ( point.y() < top + tool.width() )
    {
        top = point.y() - tool.width();
        change = true;
    }
    else if ( point.y() + tool.width() > bottom )
    {
        bottom = point.y() + tool.width();
        change = true;
    }
    if (change)
        prepareGeometryChange();
}

QList<AbstractGraphicsPath*> BasicGraphicsPath::splitErase(const QPointF &pos, const qreal size) const
{
    if (!boundingRect().marginsAdded(QMarginsF(size, size, size, size)).contains(pos))
        // If returned list contains only a NULL, this is interpreted as "no
        // changes".
        return {NULL};

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
        return {NULL};
    if (last > first + 2)
        list.append(new BasicGraphicsPath(this, first, last));
    return list;
}
