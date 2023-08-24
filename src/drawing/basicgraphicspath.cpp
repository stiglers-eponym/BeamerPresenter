// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <algorithm>
#include <QtConfig>
#include <QStringList>
#include <QWidget>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "src/log.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/drawtool.h"
#include "src/preferences.h"

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QPointF &pos) noexcept :
    AbstractGraphicsPath(tool)
{
    // Initialize bounding rect.
    const qreal tw = _tool.width();
    bounding_rect = QRectF(pos.x() - tw/2, pos.y() - tw/2, tw, tw);
    // Add first data point.
    coordinates.append(pos);
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QVector<QPointF> &coordinates, const QRectF &boundingRect) noexcept :
    AbstractGraphicsPath(tool, coordinates)
{
    if (bounding_rect.isEmpty())
    {
        shape_cache = shape();
        bounding_rect = shape_cache.boundingRect();
    }
    else
        bounding_rect = boundingRect;
}

BasicGraphicsPath::BasicGraphicsPath(const AbstractGraphicsPath * const other, int first, int last) :
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
        qWarning() << "Tried to construct a path with non-positive length";
        return;
    }

    // Copy coordinates from other.
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    coordinates = QVector<QPointF>(other->coordinates.cbegin()+first, other->coordinates.cbegin()+last);
#else
    coordinates = QVector<QPointF>(length);
    for (int i=0; i<length; i++)
        coordinates[i] = other->coordinates[i+first];
#endif
    // Cache shape and update bounding rect.
    shape_cache = shape();
    bounding_rect = shape_cache.boundingRect();
}

BasicGraphicsPath::BasicGraphicsPath(const DrawTool &tool, const QString &coordinate_string) noexcept :
    AbstractGraphicsPath(tool)
{
    debug_msg(DebugDrawing, coordinate_string);
    QStringList coordinate_list = coordinate_string.split(' ');
    // Initialize coordinates with the correct length.
    coordinates = QVector<QPointF>(coordinate_list.length()/2);
    // Read coordinates.
    int i=0;
    while (coordinate_list.length() > 1)
        coordinates[i++] = {coordinate_list.takeFirst().toDouble(), coordinate_list.takeFirst().toDouble()};
    finalize();
}

void BasicGraphicsPath::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (coordinates.isEmpty())
        return;
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(_tool.pen());
    painter->setCompositionMode(_tool.compositionMode());
    if (coordinates.length() == 1)
        painter->drawPoint(coordinates.first());
    else if (_tool.brush().style() == Qt::NoBrush)
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
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(QPen(QBrush(Qt::black), 0.5));
        painter->drawRect(bounding_rect);
        painter->drawLine(bounding_rect.topLeft(), {0,0});
        painter->drawLine(bounding_rect.topRight(), {0,0});
        painter->drawLine(bounding_rect.bottomLeft(), {0,0});
        painter->drawLine(bounding_rect.bottomRight(), {0,0});
        //painter->setPen(QPen(QBrush(Qt::red), 0.25));
        //painter->drawPath(shape());
    }
#endif
}

void BasicGraphicsPath::addPoint(const QPointF &point)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        shape_cache.clear();
#else
        shape_cache = QPainterPath();
#endif
    coordinates.append(point);
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

QList<AbstractGraphicsPath*> BasicGraphicsPath::splitErase(const QPointF &scene_pos, const qreal size) const
{
    if (!sceneBoundingRect().marginsAdded(QMarginsF(size, size, size, size)).contains(scene_pos))
        // If returned list contains only a nullptr, this is interpreted as "no
        // changes".
        return {nullptr};

    QList<AbstractGraphicsPath*> list;
    const qreal sizesq = size*size;
    int first = 0, last = 0;
    while (last < coordinates.length())
    {
        const QPointF diff = mapToScene(coordinates[last++]) - scene_pos;
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

void BasicGraphicsPath::changeTool(const DrawTool &newtool) noexcept
{
    if (!(newtool.tool() & Tool::AnyDrawTool))
    {
        qWarning() << "Cannot change draw tool to non-drawing base tool.";
        return;
    }
    if (std::abs(newtool.pen().widthF() - _tool.tool()) > 0.01)
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        shape_cache.clear();
#else
        shape_cache = QPainterPath();
#endif
    _tool.setPen(newtool.pen());
    _tool.setWidth(newtool.width());
    _tool.brush() = newtool.brush();
    _tool.setCompositionMode(newtool.compositionMode());
    // cache shape
    if (shape_cache.isEmpty())
        shape_cache = shape();
}

AbstractGraphicsPath *BasicGraphicsPath::copy() const
{
    BasicGraphicsPath *newpath = new BasicGraphicsPath(_tool, coordinates, bounding_rect);
    newpath->setPos(pos());
    newpath->setTransform(transform());
    newpath->shape_cache = shape_cache;
    return newpath;
}
