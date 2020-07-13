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

DrawPath::DrawPath(FullDrawTool const& tool, QPointF const& start) :
    tool(tool)
{
    path.append(start);
    outer = QRectF(start.x(), start.y(), 0, 0);
    updateHash();
}

DrawPath::DrawPath(FullDrawTool const& tool, QPointF const* const points, int const number) :
    path(QVector<QPointF>(number)),
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
    outer = QRectF(left, top, right-left, bottom-top);
    updateHash();
}

DrawPath::DrawPath(DrawPath const& old, QPointF const shift, double const scale) :
    path(QVector<QPointF>(old.path.length())),
    tool({old.tool.tool, old.tool.color, scale*old.tool.size, old.tool.extras}),
    hash(old.hash)
{
    for (int i=0; i<old.path.length(); i++)
        path[i] = scale*old.path[i] + shift;
    outer = QRectF(scale*old.outer.topLeft() + shift, scale*old.outer.bottomRight() + shift);
}

QRect const DrawPath::update(DrawPath const& new_path, QPointF const shift, double const scale)
{
    if (new_path.tool.tool != tool.tool || new_path.tool.color != tool.color || new_path.path.length() < path.length())
        return QRect(0,0,-1,-1);
    if (new_path.path.length() == path.length() + 1) {
        path.append(scale*new_path.path.last() + shift);
        outer = QRectF(scale*new_path.outer.topLeft() + shift, scale*new_path.outer.bottomRight() + shift);
        hash = new_path.hash;
        return QRectF(path.last(), *(path.cend()-2)).normalized()
                .adjusted(-tool.size/2-.5, -tool.size/2-.5, tool.size/2+.5, tool.size/2+.5)
                .toAlignedRect();
    }
    for (QVector<QPointF>::const_iterator it = new_path.path.cbegin() + path.length(); it != new_path.path.cend();) {
        path.append(scale*(*it++) + shift);
    }
    outer = QRectF(scale*new_path.outer.topLeft() + shift, scale*new_path.outer.bottomRight() + shift);
    hash = new_path.hash;
    return getOuterDrawing().toAlignedRect();
}

DrawPath::DrawPath(DrawPath const& old) :
    path(old.path),
    outer(old.outer),
    tool(old.tool),
    hash(old.hash)
{}

void DrawPath::transform(QPointF const& shift, const double scale)
{
    for (int i=0; i<path.length(); i++)
        path[i] = scale*path[i] + shift;
    outer = QRectF(scale*outer.topLeft() + shift, scale*outer.bottomRight() + shift);
    tool.size *= scale;
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
    return new DrawPath(tool, path.data()+start, end-start);
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

void DrawPath::toText(QStringList &stringList, QPoint const shift, qreal const scale) const
{
    for (auto point : path) {
        stringList << QString::number((point.x() - shift.x())*scale);
        stringList << QString::number((point.y() - shift.y())*scale);
    }
}

DrawPath::DrawPath(FullDrawTool const& tool, QStringList const& stringList, QPoint const shift, qreal const scale) :
    tool({tool.tool, tool.color, scale*tool.size, tool.extras})
{
    if (stringList.size() % 2 != 0 || stringList.size() < 2)
        return;
    double x = shift.x() + scale*stringList[0].toDouble();
    double y = shift.y() + scale*stringList[1].toDouble();
    double left=x, right=x, top=y, bottom=y;
    path.append(QPointF(x, y));
    for (QStringList::const_iterator it=stringList.cbegin()+2; it != stringList.cend();) {
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

QRect const DrawPath::getOuterLast() const
{
    return QRectF(path.last(), *(path.cend()-2))
            .normalized()
            .adjusted(-tool.size/2-.5, -tool.size/2-.5, tool.size/2+.5, tool.size/2+.5)
            .toAlignedRect();
}
