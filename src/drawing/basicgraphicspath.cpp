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
    coordinates.append(pos);
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QVector<QPointF> &coordinates, const QRectF &bounding_rect) noexcept :
    AbstractGraphicsPath(tool, coordinates)
{
    top = bounding_rect.top();
    bottom = bounding_rect.bottom();
    left = bounding_rect.left();
    right = bounding_rect.right();
}

BasicGraphicsPath::BasicGraphicsPath(const AbstractGraphicsPath * const other, int first, int last) :
    AbstractGraphicsPath(other->_tool)
{
    setPos(other->scenePos());
    // Make sure that first and last are valid.
    if (first < 0)
        first = 0;
    if (last > other->size() || last < 0)
        last = other->size();
    const int length = last - first;
    if (length <= 0)
        // This should never happen.
        return;

    // Initialize coordinates with the correct length.
    coordinates = QVector<QPointF>(length);
    // Initialize bounding rect.
    top = other->coordinates[first].y();
    bottom = top;
    left = other->coordinates[first].x();
    right = left;
    // Copy coordinates from other and update bounding rect.
    for (int i=0; i<length; i++)
    {
        coordinates[i] = other->coordinates[i+first];
        if ( coordinates[i].x() < left )
            left = coordinates[i].x();
        else if ( coordinates[i].x() > right )
            right = coordinates[i].x();
        if ( coordinates[i].y() < top )
            top = coordinates[i].y();
        else if ( coordinates[i].y() > bottom )
            bottom = coordinates[i].y();
    }
    // Add finite stroke width to bounding rect.
    left -= _tool.width();
    right += _tool.width();
    top -= _tool.width();
    bottom += _tool.width();
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QString &coordinate_string) noexcept :
    AbstractGraphicsPath(tool)
{
    QStringList coordinate_list = coordinate_string.split(' ');
    coordinates = QVector<QPointF>(coordinate_list.length()/2);
    qreal x, y;
    x = coordinate_list.takeFirst().toDouble();
    y = coordinate_list.takeFirst().toDouble();
    coordinates[0] = {x,y};

    // Initialize coordinates with the correct length.
    // Initialize bounding rect.
    top = y;
    bottom = y;
    left = x;
    right = x;
    // Copy coordinates from other and update bounding rect.
    int i=1;
    while (coordinate_list.length() > 1)
    {
        x = coordinate_list.takeFirst().toDouble();
        y = coordinate_list.takeFirst().toDouble();
        coordinates[i++] = {x,y};
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
    if (coordinates.isEmpty())
        return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(_tool.pen());
    painter->setCompositionMode(_tool.compositionMode());
    if (coordinates.length() == 1)
    {
        painter->drawPoint(coordinates.first());
        return;
    }

    if (_tool.brush().style() == Qt::NoBrush)
        painter->drawPolyline(coordinates.constData(), coordinates.size());
    else
    {
        painter->setBrush(_tool.brush());
        painter->drawPolygon(coordinates.constData(), coordinates.size());
    }

#ifdef QT_DEBUG
    // Show bounding box of stroke in verbose debugging mode.
    if ((preferences()->debug_level & (DebugDrawing|DebugVerbose)) == (DebugDrawing|DebugVerbose))
    {
        painter->setPen(QPen(QBrush(Qt::black), 0.5));
        painter->drawRect(boundingRect());
        painter->drawLine(left, top, 0, 0);
        painter->drawLine(left, bottom, 0, 0);
        painter->drawLine(right, top, 0, 0);
        painter->drawLine(right, bottom, 0, 0);
    }
#endif
}

void BasicGraphicsPath::addPoint(const QPointF &point)
{
    coordinates.append(point);
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
    while (last < coordinates.length())
    {
        const QPointF diff = coordinates[last++] - pos;
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
