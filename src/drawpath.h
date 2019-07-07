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

#ifndef DRAWPATH_H
#define DRAWPATH_H

#include<QVector>
#include<QPointF>
#include<QRectF>
#include<QDebug>

class DrawPath
{
private:
    QVector<QPointF> path;
    QRectF outer = QRectF();
    int eraser_size = 10;
public:
    DrawPath(QPointF const& start, int const eraser_size = 10);
    DrawPath(QPointF* const points, int const number, int const eraser_size = 10);
    DrawPath(DrawPath const& old) {path=old.path; outer=old.outer; eraser_size=old.eraser_size;}
    DrawPath(DrawPath const&& old) {path=old.path; outer=old.outer; eraser_size=old.eraser_size;}
    ~DrawPath() {path.clear();}
    void setEraserSize(int const size);
    void clear() {path.clear(); outer=QRectF();}
    bool isEmpty() {return path.isEmpty();}
    int number() const {return path.length();}
    QPointF const* data() const {return path.constData();}
    QRectF const& getOuter() const {return outer;}
    QVector<int> intersects(QPointF const& point) const;
    void append(QPointF const& point);
    DrawPath split(int start, int end);
    DrawPath operator=(DrawPath const& old) {path.clear(); return DrawPath(old);}
    DrawPath operator=(DrawPath const&& old) {path.clear(); return DrawPath(old);}
};

#endif // DRAWPATH_H
