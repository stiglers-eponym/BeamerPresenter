#include "src/drawing/abstractgraphicspath.h"

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
