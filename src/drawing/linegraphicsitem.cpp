// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPainter>
#include <QWidget>
#include <QVector>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include "src/drawing/linegraphicsitem.h"
#include "src/drawing/basicgraphicspath.h"

void LineGraphicsItem::setSecondPoint(const QPointF &pos) noexcept
{
    QLineF newline = line();
    newline.setP2(pos);
    setLine(newline);
}

BasicGraphicsPath *LineGraphicsItem::toPath() const noexcept
{
    if (line().p1() == line().p2())
        return nullptr;
    const int segments = line().length() / 10 + 2;
    const QPointF
            p1 = line().p1() - line().center(),
            delta = {line().dx()/segments, line().dy()/segments};
    QVector<QPointF> coordinates(segments+1);
    for (int i=0; i<=segments; ++i)
        coordinates[i] = p1 + i*delta;
    QRectF newrect = boundingRect();
    newrect.translate(-line().center());
    BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, newrect);
    path->setPos(mapToScene(line().center()));
    return path;
}

void LineGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setCompositionMode(tool.compositionMode());
    QGraphicsLineItem::paint(painter, option, widget);
}
