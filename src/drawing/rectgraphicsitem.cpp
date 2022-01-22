#include "src/drawing/rectgraphicsitem.h"

RectGraphicsItem::RectGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent) :
    QGraphicsRectItem(pos.x(), pos.y(), 0, 0, parent),
    tool(tool)
{
    if (tool.shape() != DrawTool::Rect)
        qWarning() << "Wrong tool given to RectGraphicsItem";
    setPen(tool.pen());
    setBrush(tool.brush());
}

void RectGraphicsItem::setSecondPoint(const QPointF &pos)
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

BasicGraphicsPath *RectGraphicsItem::toPath() const
{
    const QRectF &therect = rect();
    if (therect.isEmpty())
        return NULL;
    const int nx = therect.width() / 10 + 2,
              ny = therect.height() / 10 + 2;
    const qreal xscale = therect.width() / nx,
                yscale = therect.height() / ny,
                left = therect.left(),
                right = therect.right(),
                top = therect.top(),
                bottom = therect.bottom();
    QVector<QPointF> coordinates(2*(nx+ny)+1);
    for (int i=0; i<nx; ++i)
        coordinates[i] = {left + xscale*i, top};
    for (int i=0; i<ny; ++i)
        coordinates[nx+i] = {right, top + yscale*i};
    for (int i=0; i<nx; ++i)
        coordinates[nx+ny+i] = {right - xscale*i, bottom};
    for (int i=0; i<ny; ++i)
        coordinates[2*nx+ny+i] = {left, bottom - yscale*i};
    coordinates[2*(nx+ny)] = {left, top};
    BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, boundingRect());
    return path;
}
