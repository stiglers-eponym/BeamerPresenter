// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <algorithm>
#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QLineF>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include "src/drawing/arrowgraphicsitem.h"
#include "src/drawing/basicgraphicspath.h"

/**
 * Given the start and end point of an arrow, calculate the two other points
 * needed for an arrow.
 * @param start start point of the arrow
 * @param end end point of the arrow
 * @param p1 will be overwritten by first extra point for the arrow
 * @param p2 will be overwritten by second extra point for the arrow
 */
void calcArrowPoints(const QPointF &start, const QPointF &end, QPointF &p1, QPointF &p2)
{
    const qreal length = QLineF(start, end).length();
    const qreal scale1 = 8. / std::max(40., length) + 32. / std::max(320., length);
    const qreal scale2 = 1.2*scale1;
    p1 = {scale2*start.x() + (1-scale2)*end.x() - scale1*(end.y() - start.y()), scale2*start.y() + (1-scale2)*end.y() + scale1*(end.x() - start.x())};
    p2 = {scale2*start.x() + (1-scale2)*end.x() + scale1*(end.y() - start.y()), scale2*start.y() + (1-scale2)*end.y() - scale1*(end.x() - start.x())};
}

void ArrowGraphicsItem::setSecondPoint(const QPointF &pos)
{
    QPainterPath newpath(origin);
    newpath.lineTo(pos);
    QPointF p1, p2;
    calcArrowPoints(origin, pos, p1, p2);
    newpath.lineTo(p1);
    newpath.moveTo(p2);
    newpath.lineTo(pos);
    setPath(newpath);
}

QList<BasicGraphicsPath*> ArrowGraphicsItem::toPath() const
{
    const QPointF reference = boundingRect().center(),
            rbegin = origin - reference,
            rend = path().currentPosition() - reference;
    if (path().currentPosition().isNull() || rbegin == rend)
        return {};
    const qreal length = QLineF(rbegin, rend).length();
    const int main_segments = length / 10 + 2,
            aux_segments = length / 40 + 2;
    QPointF p1, p2;
    calcArrowPoints(rbegin, rend, p1, p2);
    qreal x = rbegin.x(),
          y = rbegin.y(),
          dx = (rend.x() - rbegin.x())/main_segments,
          dy = (rend.y() - rbegin.y())/main_segments;
    QVector<QPointF> coordinates1(main_segments+1);
    for (int i=0; i<=main_segments; ++i)
        coordinates1[i] = {x+i*dx, y+i*dy};
    auto path1 = new BasicGraphicsPath(tool, coordinates1);
    path1->setPos(mapToScene(reference));
    x = p1.x();
    y = p1.y();
    dx = (rend.x() - p1.x())/aux_segments;
    dy = (rend.y() - p1.y())/aux_segments;
    QVector<QPointF> coordinates2(2*aux_segments+1);
    for (int i=0; i<aux_segments; ++i)
        coordinates2[i] = {x+i*dx, y+i*dy};
    x = rend.x();
    y = rend.y();
    dx = (p2.x() - rend.x())/aux_segments;
    dy = (p2.y() - rend.y())/aux_segments;
    for (int i=0; i<=aux_segments; ++i)
        coordinates2[aux_segments + i] = {x+i*dx, y+i*dy};
    auto path2 = new BasicGraphicsPath(tool, coordinates2);
    path2->setPos(mapToScene(reference));
    return {path1,path2};
}

void ArrowGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setCompositionMode(tool.compositionMode());
    QGraphicsPathItem::paint(painter, option, widget);
}
