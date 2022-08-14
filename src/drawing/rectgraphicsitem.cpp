// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QRectF>
#include <QVector>
#include "src/drawing/rectgraphicsitem.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/log.h"

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
        origin ^= OriginRight;
    if (newrect.height() < 0)
        origin ^= OriginBottom;
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
                bottom = therect.bottom(),
                rleft = (left - right)/2,
                rtop = (top - bottom)/2;
    QVector<QPointF> coordinates(2*(nx+ny)+1);
    for (int i=0; i<nx; ++i)
        coordinates[i] = {rleft + xscale*i, rtop};
    for (int i=0; i<ny; ++i)
        coordinates[nx+i] = {-rleft, rtop + yscale*i};
    for (int i=0; i<nx; ++i)
        coordinates[nx+ny+i] = {-rleft - xscale*i, -rtop};
    for (int i=0; i<ny; ++i)
        coordinates[2*nx+ny+i] = {rleft, -rtop - yscale*i};
    coordinates[2*(nx+ny)] = {rleft, rtop};
    QRectF newrect = boundingRect();
    newrect.translate(-newrect.center());
    BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, newrect);
    path->setPos(mapToScene((left+right)/2, (top+bottom)/2));
    return path;
}
