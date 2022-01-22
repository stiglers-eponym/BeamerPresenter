#include <QPainter>
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/rectgraphicsitem.h"

#ifdef QT_DEBUG
#include "src/preferences.h"
#endif

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QPointF &pos) noexcept :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    top = pos.y() - _tool.width();
    bottom = pos.y() + _tool.width();
    left = pos.x() - _tool.width();
    right = pos.x() + _tool.width();
    // Add first data point.
    data.append(pos);
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QVector<QPointF> &coordinates, const QRectF &bounding_rect) noexcept :
    AbstractGraphicsPath(tool),
    data(coordinates)
{
    top = bounding_rect.top();
    bottom = bounding_rect.bottom();
    left = bounding_rect.left();
    right = bounding_rect.right();
}

BasicGraphicsPath::BasicGraphicsPath(const BasicGraphicsPath * const other, int first, int last) :
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
    left -= _tool.width();
    right += _tool.width();
    top -= _tool.width();
    bottom += _tool.width();
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QString &coordinates) noexcept :
    AbstractGraphicsPath(tool)
{
    QStringList coordinate_list = coordinates.split(' ');
    data = QVector<QPointF>(coordinate_list.length()/2);
    qreal x, y;
    x = coordinate_list.takeFirst().toDouble();
    y = coordinate_list.takeFirst().toDouble();
    data[0] = {x,y};

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
        data[i++] = {x,y};
        if ( x < left )
            left = x;
        else if ( x > right )
            right = x;
        if ( y < top )
            top = y;
        else if ( y > bottom )
            bottom = y;
    }
    // Add finite stroke width to bounding rect.
    left -= _tool.width();
    right += _tool.width();
    top -= _tool.width();
    bottom += _tool.width();
}

void BasicGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (data.isEmpty())
        return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(_tool.pen());
    painter->setCompositionMode(_tool.compositionMode());
    if (data.length() == 1)
    {
        painter->drawPoint(data.first());
        return;
    }

    if (_tool.brush().style() == Qt::NoBrush)
        painter->drawPolyline(data.constData(), data.size());
    else
    {
        painter->setBrush(_tool.brush());
        painter->drawPolygon(data.constData(), data.size());
    }

#ifdef QT_DEBUG
    // Show bounding box of stroke in verbose debugging mode.
    if ((preferences()->debug_level & (DebugDrawing|DebugVerbose)) == (DebugDrawing|DebugVerbose))
    {
        painter->setPen(QPen(QBrush(Qt::black), 0.5));
        painter->drawRect(boundingRect());
    }
#endif
}

void BasicGraphicsPath::addPoint(const QPointF &point)
{
    data.append(point);
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

void BasicGraphicsPath::changeTool(const DrawTool &newtool) noexcept
{
    if (newtool.tool() != _tool.tool())
    {
        qWarning() << "Cannot change tool to different base tool.";
        return;
    }
    _tool.setPen(newtool.pen());
    _tool.setWidth(newtool.width());
    _tool.setCompositionMode(newtool.compositionMode());
}

const QString BasicGraphicsPath::stringCoordinates() const noexcept
{
    QString str;
    for (const auto point : data)
    {
        str += QString::number(point.x());
        str += ' ';
        str += QString::number(point.y());
        str += ' ';
    }
    str.chop(1);
    return str;
}
