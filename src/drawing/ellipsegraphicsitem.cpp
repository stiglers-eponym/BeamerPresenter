#include <cmath>
#include "ellipsegraphicsitem.h"

EllipseGraphicsItem::EllipseGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent) :
    QGraphicsEllipseItem(pos.x(), pos.y(), 0, 0, parent),
    tool(tool)
{
    if (tool.shape() != DrawTool::Ellipse)
        qWarning() << "Wrong tool given to EllipseGraphicsItem";
    setPen(tool.pen());
    setBrush(tool.brush());
}

void EllipseGraphicsItem::setSecondPoint(const QPointF &pos)
{
    QRectF newrect = rect();
    switch (origin)
    {
    case TopLeft:
        newrect.setBottomRight(pos);
        break;
    case TopRight:
        newrect.setBottomLeft(pos);
        break;
    case BottomLeft:
        newrect.setTopRight(pos);
        break;
    case BottomRight:
        newrect.setTopLeft(pos);
        break;
    }
    if (newrect.width() < 0)
        origin ^= 0x1;
    if (newrect.height() < 0)
        origin ^= 0x2;
    setRect(newrect.normalized());
}

BasicGraphicsPath *EllipseGraphicsItem::toPath() const
{
    const QRectF &therect = rect();
    int segments = (therect.width() + therect.height()) / 3 + 10;
    qreal rx = therect.width() / 2,
          ry = therect.height() / 2,
          phasestep = 2*M_PI / segments,
          cx = therect.center().x(),
          cy = therect.center().y();
    QVector<QPointF> coordinates(segments+1);
    for (int i=0; i<segments; ++i)
        coordinates[i] = {cx + rx*std::sin(phasestep*i), cy + ry*std::cos(phasestep*i)};
    coordinates[segments] = {cx, cy + ry};
    BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, boundingRect());
    return path;
}
