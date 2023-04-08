// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QtConfig>
#include <QWidget>
#include <QPainter>
#include <QLineF>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include "src/drawing/fullgraphicspath.h"
#include "src/preferences.h"
#include "src/log.h"

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QPointF &pos, const float pressure) :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    const qreal tw = _tool.width();
    bounding_rect = QRectF(pos.x() - tw/2, pos.y() - tw/2, tw, tw);
    // Add first data point.
    coordinates.append({pos.x(), pos.y()});
    pressures.append(_tool.width()*pressure);
}

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QVector<QPointF> &coordinates, const QVector<float> &pressures) :
    AbstractGraphicsPath(tool, coordinates),
    pressures(pressures)
{
    shape_cache = shape();
    bounding_rect = shape_cache.boundingRect();
}

FullGraphicsPath::FullGraphicsPath(const FullGraphicsPath * const other, int first, int last) :
    AbstractGraphicsPath(other->_tool)
{
    setPos(other->pos());
    setTransform(other->transform());
    // Make sure that first and last are valid.
    if (first < 0)
        first = 0;
    if (last > other->size() || last < 0)
        last = other->size();
    const int length = last - first;
    if (length <= 0)
    {
        // This should never happen.
        warn_msg("Tried to construct a path with non-positive length");
        return;
    }
    // Copy data.
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    pressures = QVector<float>(other->pressures.cbegin()+first, other->pressures.cbegin()+last);
    coordinates = QVector<QPointF>(other->coordinates.cbegin()+first, other->coordinates.cbegin()+last);
#else
    pressures = QVector<float>(length);
    coordinates = QVector<QPointF>(length);
    // Copy data points from other and update bounding rect.
    for (int i=0; i<length; i++)
    {
        coordinates[i] = other->coordinates[i+first];
        pressures[i] = other->pressures[i+first];
    }
#endif
    // Cache shape and update bounding rect.
    shape_cache = shape();
    bounding_rect = shape_cache.boundingRect();
}

FullGraphicsPath::FullGraphicsPath(const DrawTool &tool, const QString &coordinate_string, const QString &weights) :
    AbstractGraphicsPath(tool)
{
    QStringList coordinate_list = coordinate_string.split(' ');
    QStringList weight_list = weights.split(' ');

    // Initialize vectors with the correct length.
    coordinates = QVector<QPointF>(coordinate_list.length()/2);
    pressures = QVector<float>(coordinate_list.length()/2);
    float w=tool.width(), max_weight = 0;

    // Copy data points from other and update bounding rect.
    int i=0;
    while (coordinate_list.length() > 1)
    {
        coordinates[i] = {coordinate_list.takeFirst().toDouble(), coordinate_list.takeFirst().toDouble()};
        if (!weight_list.isEmpty())
        {
            w = weight_list.takeFirst().toFloat();
            if (w > max_weight)
                max_weight = w;
        }
        pressures[i++] = w;
    }
    max_weight *= 1.05;
    _tool.setWidth(max_weight);
    finalize();
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
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(QPen(QBrush(Qt::black), 0.5));
        painter->drawRect(boundingRect());
        painter->drawLine(bounding_rect.topLeft(), {0,0});
        painter->drawLine(bounding_rect.topRight(), {0,0});
        painter->drawLine(bounding_rect.bottomLeft(), {0,0});
        painter->drawLine(bounding_rect.bottomRight(), {0,0});
    }
#endif
}

void FullGraphicsPath::addPoint(const QPointF &point, const float pressure)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        shape_cache.clear();
#else
        shape_cache = QPainterPath();
#endif
    coordinates.append(point);
    pressures.append(_tool.width()*pressure);
    bool change = false;
    if ( point.x() < bounding_rect.left() + _tool.width()*0.55 )
    {
        bounding_rect.setLeft(point.x() - _tool.width()*0.55);
        change = true;
    }
    else if ( point.x() + _tool.width()*0.55 > bounding_rect.right() )
    {
        bounding_rect.setRight(point.x() + _tool.width()*0.55);
        change = true;
    }
    if ( point.y() < bounding_rect.top() + _tool.width()*0.55 )
    {
        bounding_rect.setTop(point.y() - _tool.width()*0.55);
        change = true;
    }
    else if ( point.y() + _tool.width()*0.55 > bounding_rect.bottom() )
    {
        bounding_rect.setBottom(point.y() + _tool.width()*0.55);
        change = true;
    }
    if (change)
        prepareGeometryChange();
}

QList<AbstractGraphicsPath*> FullGraphicsPath::splitErase(const QPointF &scene_pos, const qreal size) const
{
    if (!sceneBoundingRect().marginsAdded(QMarginsF(size, size, size, size)).contains(scene_pos))
        // If returned list contains only a NULL, this is interpreted as "no
        // changes".
        return {NULL};

    QList<AbstractGraphicsPath*> list;
    const qreal sizesq = size*size;
    int first = 0, last = 0;
    while (last < coordinates.length())
    {
        const QPointF diff = mapToScene(coordinates[last++]) - scene_pos;
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
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        shape_cache.clear();
#else
        shape_cache = QPainterPath();
#endif
    const float scale = newwidth / _tool.width();
    auto it = pressures.begin();
    while (++it != pressures.end())
        *it *= scale;
    // cache shape
    shape_cache = shape();
}

void FullGraphicsPath::changeTool(const DrawTool &newtool) noexcept
{
    if (!(newtool.tool() & Tool::AnyDrawTool))
    {
        qWarning() << "Cannot change draw tool to non-drawing base tool.";
        return;
    }
    const float newwidth = newtool.width();
    if (newwidth != _tool.width())
        changeWidth(newwidth);
    _tool.setPen(newtool.pen());
    _tool.setCompositionMode(newtool.compositionMode());
    // cache shape
    shape_cache = shape();
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

AbstractGraphicsPath *FullGraphicsPath::copy() const
{
    FullGraphicsPath *newpath = new FullGraphicsPath(_tool, coordinates.first(), pressures.first());
    newpath->coordinates = coordinates;
    newpath->pressures = pressures;
    newpath->bounding_rect = bounding_rect;
    newpath->setPos(pos());
    newpath->setTransform(transform());
    newpath->shape_cache = shape_cache;
    return newpath;
}
