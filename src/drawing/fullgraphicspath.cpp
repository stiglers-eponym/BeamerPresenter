#include <QPainter>
#include "src/drawing/fullgraphicspath.h"

#ifdef QT_DEBUG
#include "src/preferences.h"
#endif

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QPointF &pos, const float pressure) :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    top = pos.y() - _tool.width();
    bottom = pos.y() + _tool.width();
    left = pos.x() - _tool.width();
    right = pos.x() + _tool.width();
    // Add first data point.
    coordinates.append({pos.x(), pos.y()});
    pressures.append(_tool.width()*pressure);
}

FullGraphicsPath::FullGraphicsPath(const FullGraphicsPath * const other, int first, int last) :
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
    // Initialize data with the correct length.
    coordinates = QVector<QPointF>(length);
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    pressures = QVector<float>(other->pressures.cbegin()+first, other->pressures.cbegin()+last);
#else
    // In Qt < 5.14 this vector is only created here and filled later:
    pressures = QVector<float>(length);
#endif
    // Initialize bounding rect.
    top = other->coordinates[first].y();
    bottom = top;
    left = other->coordinates[first].x();
    right = left;
    // Copy data points from other and update bounding rect.
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
#if (QT_VERSION < QT_VERSION_CHECK(5,14,0))
        // In Qt >= 5.14 this vector has already been filled (directly copied).
        pressures[i] = other->pressures[i+first];
#endif
    }

    // Add finite stroke width to bounding rect.
    left -= _tool.width();
    right += _tool.width();
    top -= _tool.width();
    bottom += _tool.width();
}

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QString &coordinate_string, const QString &weights) :
    AbstractGraphicsPath(tool)
{
    QStringList coordinate_list = coordinate_string.split(' ');
    QStringList weight_list = weights.split(' ');

    // Initialize vectors with the correct length.
    coordinates = QVector<QPointF>(coordinate_list.length()/2);
    pressures = QVector<float>(coordinate_list.length()/2);
    qreal x, y;
    float w, max_weight;
    x = coordinate_list.takeFirst().toDouble();
    y = coordinate_list.takeFirst().toDouble();
    w = weight_list.takeFirst().toFloat();
    max_weight = w;
    coordinates[0] = {x, y};
    pressures[0] = w;

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
        coordinates[i] = {x, y};
        pressures[i++] = w;
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
    if (coordinates.isEmpty())
        return;
    if (coordinates.length() == 1)
    {
        QPen pen = _tool.pen();
        pen.setWidthF(pressures.first());
        painter->setPen(pen);
        painter->drawPoint(coordinates.first());
        return;
    }
    if (_tool.brush().style() != Qt::NoBrush)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(_tool.brush());
        painter->drawPolygon(coordinates.constData(), coordinates.size());
    }
    QPen pen = _tool.pen();
    auto cit = coordinates.cbegin();
    auto pit = pressures.cbegin();
    if (pen.style() == Qt::SolidLine)
    {
        while (++cit != coordinates.cend())
        {
            pen.setWidthF(*++pit);
            painter->setPen(pen);
            painter->drawLine(*(cit-1), *cit);
        }
    }
    else if (pen.style() != Qt::NoPen)
    {
        qreal len = 0;
        QLineF line;
        while (++cit != coordinates.cend())
        {
            pen.setWidthF(*++pit);
            pen.setDashOffset(len);
            painter->setPen(pen);
            line = QLineF(*(cit-1), *cit);
            painter->drawLine(line);
            len += line.length();
        }
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

void FullGraphicsPath::addPoint(const QPointF &point, const float pressure)
{
    coordinates.append({point.x(), point.y()});
    pressures.append(_tool.width()*pressure);
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
    while (last < coordinates.length())
    {
        const QPointF diff = coordinates[last++] - pos;
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
    auto it = pressures.begin();
    while (++it != pressures.cend())
        *it *= scale;
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

const QString FullGraphicsPath::stringWidth() const noexcept
{
    QString str;
    for (const auto pr : pressures)
    {
        str += QString::number(pr);
        str += ' ';
    }
    str.chop(1);
    return str;
}
