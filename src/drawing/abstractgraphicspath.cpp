#include "src/drawing/abstractgraphicspath.h"
#include "src/preferences.h"

const QString AbstractGraphicsPath::stringCoordinates() const noexcept
{
    QString str;
    const qreal x = scenePos().x(), y = scenePos().y();
    for (const auto &point : coordinates)
    {
        str += QString::number(point.x() + x);
        str += ' ';
        str += QString::number(point.y() + y);
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
    // TODO: change width
    const QPointF new_scene_pos = mapToScene((left+right)/2, (top+bottom)/2);
    for (auto &point : coordinates)
        point = mapToScene(point) - new_scene_pos;
    resetTransform();
    setPos(new_scene_pos);
    shape_cache = shape();
    const QRectF rect = shape_cache.boundingRect();
    left = rect.left();
    right = rect.right();
    top = rect.top();
    bottom = rect.bottom();
}
