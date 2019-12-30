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

void DrawPath::setEraserSize(const quint16 size)
{
    if (eraser_size < 1) {
        qWarning() << "Setting an eraser size < 1 does not make any sense. Ignoring this.";
        return;
    }
    eraser_size = size;
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QPointF const& start, quint16 const eraser_size)
{
    path.append(start);
    outer = QRectF(start.x(), start.y(), 0, 0);
    this->eraser_size = eraser_size;
    this->tool = tool;
    updateHash();
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QPointF* const points, int const number, quint16 const eraser_size) :
    path(QVector<QPointF>(number)),
    eraser_size(eraser_size),
    tool(tool)
{
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
    updateHash();
}

DrawPath::DrawPath(DrawPath const& old, QPointF const shift, double const scale) :
    path(QVector<QPointF>(old.path.length())),
    tool(old.tool),
    hash(old.hash)
{
    for (int i=0; i<old.path.length(); i++)
        path[i] = scale*old.path[i] + shift;
    outer = QRectF(scale*old.outer.topLeft() + shift, scale*old.outer.bottomRight() + shift);
    eraser_size = quint16(scale*old.eraser_size+0.5);
    //eraser_size(old.eraser_size),
}

bool DrawPath::update(DrawPath const& new_path, QPointF const shift, double const scale)
{
    if (new_path.tool.tool != tool.tool || new_path.tool.color != tool.color || new_path.path.length() < path.length())
        return false;
    for (int i=path.length(); i<new_path.path.length();)
        path.append(scale*new_path.path[i++] + shift);
    outer = QRectF(scale*new_path.outer.topLeft() + shift, scale*new_path.outer.bottomRight() + shift);
    hash = new_path.hash;
    return true;
}

DrawPath::DrawPath(DrawPath const& old) :
    path(old.path),
    outer(old.outer),
    eraser_size(old.eraser_size),
    tool(old.tool),
    hash(old.hash)
{}

void DrawPath::transform(QPointF const&shift, const double scale)
{
     for (int i=0; i<path.length(); i++)
         path[i] = scale*path[i] + shift;
     outer = QRectF(scale*outer.topLeft() + shift, scale*outer.bottomRight() + shift);
     //eraser_size = static_cast<int>(scale*eraser_size+0.5);
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
    hash ^= quint32(std::hash<double>{}(point.x() + 1e5*point.y())) + (hash << 6) + (hash >> 2);
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

DrawPath* DrawPath::split(int start, int end)
{
    if (start < 0) {
        if (end >= path.length())
            return this;
        start = 0;
    }
    if (end > path.length())
        end = path.length();
    return new DrawPath(tool, path.data()+start, end-start, eraser_size);
}

void DrawPath::updateHash()
{
    hash = quint32(std::hash<int>{}(tool.tool));
    hash ^= quint32(tool.color.red())   + (hash << 6) + (hash >> 2);
    hash ^= quint32(tool.color.green()) + (hash << 6) + (hash >> 2);
    hash ^= quint32(tool.color.blue())  + (hash << 6) + (hash >> 2);
    hash ^= quint32(tool.color.alpha()) + (hash << 6) + (hash >> 2);
    for (auto& p : path)
        hash ^= quint32(std::hash<double>{}(p.x() + 1e5*p.y())) + (hash << 6) + (hash >> 2);
}

void DrawPath::toIntVector(QVector<float>& vec, int const xshift, int const yshift, int const width, int const height) const
{
    for (auto point : path) {
        vec.append(static_cast<float>(point.x() - xshift)/width);
        vec.append(static_cast<float>(point.y() - yshift)/height);
    }
}


DrawPath::DrawPath(ColoredDrawTool const& tool, QVector<float> const& vec, int const xshift, int const yshift, int const width, int const height, quint16 const eraser_size)
{
    this->eraser_size = eraser_size;
    this->tool = tool;
    path = QVector<QPointF>();
    double left=width+2*xshift, right=0, top=height+2*yshift, bottom=0;
    double x, y;
    for (QVector<float>::const_iterator it=vec.cbegin(); it!=vec.cend();) {
        x = xshift + width*static_cast<qreal>(*it++);
        y = yshift + height*static_cast<qreal>(*it++);
        path.append(QPointF(x, y));
        if (left > x)
            left = x;
        else if (right < x)
            right = x;
        if (bottom < y)
            bottom = y;
        else if (top > y)
            top = y;
    }
    outer = QRectF(left-eraser_size, top-eraser_size, right-left+2*eraser_size, bottom-top+2*eraser_size);
    updateHash();
}
