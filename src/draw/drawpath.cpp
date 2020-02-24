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

qreal square(qreal const a)
{
    return a*a;
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QPointF const& start, qreal const size) :
    tool(tool),
    size(size)
{
    path.append(start);
    outer = QRectF(start.x(), start.y(), 0, 0);
    updateHash();
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QPointF const* const points, int const number, qreal const size) :
    path(QVector<QPointF>(number)),
    tool(tool),
    size(size)
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
    outer = QRectF(left, top, right-left, bottom-top);
    updateHash();
}

DrawPath::DrawPath(DrawPath const& old, QPointF const shift, double const scale) :
    path(QVector<QPointF>(old.path.length())),
    tool(old.tool),
    size(scale*old.size),
    hash(old.hash)
{
    for (int i=0; i<old.path.length(); i++)
        path[i] = scale*old.path[i] + shift;
    outer = QRectF(scale*old.outer.topLeft() + shift, scale*old.outer.bottomRight() + shift);
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
    tool(old.tool),
    size(old.size),
    hash(old.hash)
{}

void DrawPath::transform(QPointF const& shift, const double scale)
{
    for (int i=0; i<path.length(); i++)
        path[i] = scale*path[i] + shift;
    outer = QRectF(scale*outer.topLeft() + shift, scale*outer.bottomRight() + shift);
    size *= scale;
}

void DrawPath::append(QPointF const& point)
{
    if (point.x() < outer.left())
        outer.setLeft(point.x());
    else if (point.x() > outer.right())
        outer.setRight(point.x());
    if (point.y() < outer.top())
        outer.setTop(point.y());
    else if (point.y() > outer.bottom())
        outer.setBottom(point.y());
    path.append(point);
    hash ^= quint32(std::hash<double>{}(point.x() + 1e5*point.y())) + (hash << 6) + (hash >> 2);
}

QVector<int> DrawPath::intersects(QPointF const& point, const qreal eraser_size) const
{
     if (!(outer.adjusted(-eraser_size, -eraser_size, eraser_size, eraser_size).contains(point)))
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
    return new DrawPath(tool, path.data()+start, end-start, size);
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
    // Deprecate
    for (auto point : path) {
        vec.append(static_cast<float>(point.x() - xshift)/width);
        vec.append(static_cast<float>(point.y() - yshift)/height);
    }
}

void DrawPath::toText(QStringList &stringList, QPoint const shift, qreal const scale) const
{
    for (auto point : path) {
        stringList << QString::number((point.x() - shift.x())*scale);
        stringList << QString::number((point.y() - shift.y())*scale);
    }
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QVector<float> const& vec, int const xshift, int const yshift, int const width, int const height, qreal const size) :
    tool(tool),
    size(size)
{
    // Deprecated
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
    outer = QRectF(left, top, right-left, bottom-top);
    updateHash();
}

DrawPath::DrawPath(ColoredDrawTool const& tool, QStringList const& stringList, QPoint const shift, qreal const scale, qreal const size) :
    tool(tool),
    size(scale*size)
{
    if (stringList.size() % 2 != 0 || stringList.size() < 2)
        return;
    double x = shift.x() + scale*stringList[0].toDouble();
    double y = shift.y() + scale*stringList[1].toDouble();
    double left=x, right=x, top=y, bottom=y;
    for (QStringList::const_iterator it=stringList.cbegin()+2; it != stringList.end();) {
        x = shift.x() + scale*(it++)->toDouble();
        y = shift.y() + scale*(it++)->toDouble();
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
    outer = QRectF(left, top, right-left, bottom-top);
    updateHash();
}

void DrawPath::endDrawing()
{
    if (path.length() == 1)
        path.append(path[0] + QPointF(1e-6, 0));
}
