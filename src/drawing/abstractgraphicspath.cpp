// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/abstractgraphicspath.h"
#include "src/preferences.h"
#include "src/log.h"

const QString AbstractGraphicsPath::stringCoordinates() const noexcept
{
    QString str;
    QPointF scene_point;
    for (const auto &point : coordinates)
    {
        scene_point = mapToScene(point);
        str += QString::number(scene_point.x());
        str += ' ';
        str += QString::number(scene_point.y());
        str += ' ';
    }
    /* Xournalpp cannot handle strokes consisting of a single point.
     * To ensure compatibility, a single point is simply repeated. */
    if (coordinates.length() == 1)
    {
        str += QString::number(scene_point.x());
        str += ' ';
        str += QString::number(scene_point.y());
    }
    else
        str.chop(1);
    return str;
}

const QString AbstractGraphicsPath::svgCoordinates() const noexcept
{
    QString str = "M";
    for (const auto &point : coordinates)
    {
        str += ' ';
        str += QString::number(point.x());
        str += ',';
        str += QString::number(point.y());
    }
    return str;
}

QPainterPath AbstractGraphicsPath::shape() const
{
    if (!shape_cache.isEmpty() || coordinates.isEmpty())
        return shape_cache;
    debug_verbose(DebugDrawing, "calculating shape" << coordinates.length() << this << bounding_rect);
    QPainterPath path;
    if (coordinates.length() == 1)
    {
        const qreal radius = _tool.width()/2;
        path.addEllipse(coordinates.first(), radius, radius);
        return path;
    }
    path.addPolygon(QPolygonF(coordinates));
    QPen pen(_tool.pen());
    if (pen.widthF() < preferences()->path_min_selectable_width)
        pen.setWidthF(preferences()->path_min_selectable_width);
    return QPainterPathStroker(pen).createStroke(path);
}

void AbstractGraphicsPath::finalize()
{
    // TODO: change width for scaled paths
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
    shape_cache.clear();
#else
    shape_cache = QPainterPath();
#endif
    const QPointF new_scene_pos = mapToScene(bounding_rect.center());
    for (auto &point : coordinates)
        point = mapToScene(point) - new_scene_pos;
    resetTransform();
    setPos(new_scene_pos);
    shape_cache = shape();
    bounding_rect = shape_cache.controlPointRect();
}
