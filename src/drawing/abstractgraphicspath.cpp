#include "src/drawing/abstractgraphicspath.h"
#include "src/preferences.h"

const QString AbstractGraphicsPath::stringCoordinates() const noexcept
{
    QString str;
    const qreal x = scenePos().x(), y = scenePos().y();
    QPointF scene_point;
    for (const auto &point : coordinates)
    {
        scene_point = mapToScene(point);
        str += QString::number(scene_point.x() + x);
        str += ' ';
        str += QString::number(scene_point.y() + y);
        str += ' ';
    }
    str.chop(1);
    return str;
}

QPainterPath AbstractGraphicsPath::shape() const
{
    if (!shape_cache.isEmpty())
        return shape_cache;
    QPainterPath path;
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
    bounding_rect = shape_cache.boundingRect();
}
