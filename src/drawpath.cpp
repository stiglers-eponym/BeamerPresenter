/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "drawpath.h"

double square(double const a)
{
    return a*a;
}

DrawPath::DrawPath(QPointF const& start)
{
    path = QVector<QPointF>();
    path.append(start);
    outer = QRectF(start.x(), start.y(), 0, 0);
}

DrawPath::DrawPath(QPointF* const points, int const number)
{
    path = QVector<QPointF>(number);
    if (number == 0)
        return;
    double left=points[0].x(), right=points[0].x(), top=points[0].y(), bottom=points[0].y();
    for (int i=0; i<number; i++) {
        path[i] = points[i];
        if (left > points[i].x())
            left = points[i].x();
        else if (right < points[i].x())
            right = points[i].x();
        if (bottom < points[i].y())
            bottom = points[i].y();
        else if (top > points[i].y())
            top = points[i].y();
    }
    outer = QRectF(left-eraser_size, top-eraser_size, right-left+2*eraser_size, bottom-top+2*eraser_size);
}

void DrawPath::append(QPointF const& point)
{
    path.append(point);
    if (!outer.contains(point)) {
        if (point.x() < outer.left()+eraser_size)
            outer.setLeft(point.x()-eraser_size);
        else if (point.x()+eraser_size > outer.right())
            outer.setRight(point.x()+eraser_size);
        if (point.y() < outer.top()+eraser_size)
            outer.setTop(point.y()-eraser_size);
        else if (point.y()+eraser_size > outer.bottom())
            outer.setBottom(point.y()+eraser_size);
    }
}

QVector<int> DrawPath::intersects(QPointF const& point) const
{
     if (!outer.contains(point))
         return QVector<int>();
     QVector<int> vec = QVector<int>();
     for (int i=0; i<path.length(); i++) {
         if (abs(point.x() - path[i].x()) < eraser_size
                 && abs(point.y() - path[i].y()) < eraser_size
                 && square(point.x() - path[i].x()) + square(point.y() - path[i].y()) < square(eraser_size))
             vec.append(i);
     }
     return vec;
}

DrawPath DrawPath::split(int start, int end)
{
    if (start < 0) {
        if (end >= path.length())
            return *this;
        start = 0;
    }
    if (end > path.length())
        end = path.length();
    return DrawPath(path.data()+start, end-start);
}
