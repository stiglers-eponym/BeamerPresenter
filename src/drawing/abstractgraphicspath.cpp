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
