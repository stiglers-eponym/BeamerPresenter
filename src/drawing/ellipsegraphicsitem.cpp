// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

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
        origin ^= OriginRight;
    if (newrect.height() < 0)
        origin ^= OriginBottom;
    setRect(newrect.normalized());
}

BasicGraphicsPath *EllipseGraphicsItem::toPath() const
{
    const QRectF &therect = rect();
    if (therect.isEmpty())
        return NULL;
    const int segments = (therect.width() + therect.height()) / 3 + 10;
    const qreal rx = therect.width() / 2,
                ry = therect.height() / 2,
                rx_plus_pen = rx + pen().widthF()/2,
                ry_plus_pen = ry + pen().widthF()/2,
                phasestep = 2*M_PI / segments,
                cx = therect.center().x(),
                cy = therect.center().y();
    QVector<QPointF> coordinates(segments+1);
    for (int i=0; i<segments; ++i)
        coordinates[i] = {rx*std::sin(phasestep*i), ry*std::cos(phasestep*i)};
    coordinates[segments] = {0, ry};
    BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, QRectF(-rx_plus_pen, -ry_plus_pen, 2*rx_plus_pen, 2*ry_plus_pen));
    path->setPos(mapToScene(cx, cy));
    return path;
}
