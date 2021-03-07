#include "src/drawing/fullgraphicspath.h"
#include <QPainter>

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QPointF &pos, const float pressure) :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    top = pos.y() - _tool.width();
    bottom = pos.y() + _tool.width();
    left = pos.x() - _tool.width();
    right = pos.x() + _tool.width();
    // Add first data point.
    data.append({pos, _tool.width()*pressure});
}

FullGraphicsPath::FullGraphicsPath(const FullGraphicsPath * const other, int first, int last) :
    AbstractGraphicsPath(other->_tool)
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
    left -= _tool.width();
    right += _tool.width();
    top -= _tool.width();
    bottom += _tool.width();
}

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QString &coordinates, const QString &weights) :
    AbstractGraphicsPath(tool)
{
    QStringList coordinate_list = coordinates.split(' ');
    QStringList weight_list = weights.split(' ');
    data = QVector<PointPressure>(coordinate_list.length()/2);
    qreal x, y;
    float w, max_weight;
    x = coordinate_list.takeFirst().toDouble();
    y = coordinate_list.takeFirst().toDouble();
    w = weight_list.takeFirst().toFloat();
    max_weight = w;
    data[0] = {{x,y},w};

    // Initialize data with the correct length.
    // Initialize bounding rect.
    top = y;
    bottom = y;
    left = x;
    right = x;
    // Copy data points from other and update bounding rect.
    int i=1;
    while (coordinate_list.length() > 1)
    {
        x = coordinate_list.takeFirst().toDouble();
        y = coordinate_list.takeFirst().toDouble();
        if (!weight_list.isEmpty())
        {
            w = weight_list.takeFirst().toFloat();
            if (w > max_weight)
                max_weight = w;
        }
        data[i++] = {{x,y},w};
        if ( x < left )
            left = x;
        else if ( x > right )
            right = x;
        if ( y < top )
            top = y;
        else if ( y > bottom )
            bottom = y;
    }

    max_weight *= 1.05;
    _tool.setWidth(max_weight);
    // Add finite stroke width to bounding rect.
    left -= max_weight;
    right += max_weight;
    top -= max_weight;
    bottom += max_weight;
}

void FullGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (data.isEmpty())
        return;
    QPen pen = _tool.pen();
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
    data.append({point, _tool.width()*pressure});
    bool change = false;
    if ( point.x() < left + _tool.width() )
    {
        left = point.x() - _tool.width();
        change = true;
    }
    else if ( point.x() + _tool.width() > right )
    {
        right = point.x() + _tool.width();
        change = true;
    }
    if ( point.y() < top + _tool.width() )
    {
        top = point.y() - _tool.width();
        change = true;
    }
    else if ( point.y() + _tool.width() > bottom )
    {
        bottom = point.y() + _tool.width();
        change = true;
    }
    if (change)
        prepareGeometryChange();
}

QList<AbstractGraphicsPath*> FullGraphicsPath::splitErase(const QPointF &pos, const qreal size) const
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
        return {NULL};
    if (last > first + 2)
        list.append(new FullGraphicsPath(this, first, last));
    return list;
}

void FullGraphicsPath::changeWidth(const float newwidth) noexcept
{
    const float scale = newwidth / _tool.width();
    auto it = data.begin();
    while (++it != data.cend())
        it->pressure *= scale;
}

void FullGraphicsPath::changeTool(const DrawTool &newtool) noexcept
{
    if (newtool.tool() != _tool.tool())
    {
        qWarning() << "Cannot change tool to different base tool.";
        return;
    }
    const float newwidth = newtool.width();
    if (newwidth != _tool.width())
        changeWidth(newwidth);
    _tool.setPen(newtool.pen());
    _tool.setCompositionMode(newtool.compositionMode());
}

const QString FullGraphicsPath::stringCoordinates() const noexcept
{
    QString str;
    for (const auto point : data)
    {
        str += QString::number(point.point.x());
        str += ' ';
        str += QString::number(point.point.y());
        str += ' ';
    }
    str.chop(1);
    return str;
}

const QString FullGraphicsPath::stringWidth() const noexcept
{
    QString str;
    for (const auto point : data)
    {
        str += QString::number(point.pressure);
        str += ' ';
    }
    str.chop(1);
    return str;
}
