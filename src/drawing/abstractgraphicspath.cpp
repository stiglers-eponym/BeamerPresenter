#include "src/drawing/abstractgraphicspath.h"
#include "src/preferences.h"

void AbstractGraphicsPath::toCenterCoordinates()
{
    const qreal xshift = -(left+right)/2,
                yshift = -(top+bottom)/2;
    left += xshift;
    right += xshift;
    top += yshift;
    bottom += yshift;
    const QPointF shift(xshift, yshift);
    for (auto &point : coordinates)
        point += shift;
    setPos(pos() - shift);
}

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
    toCenterCoordinates();
    shape_cache = shape();
}
