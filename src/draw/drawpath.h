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

#include <QtDebug>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include "../enumerates.h"

class DrawPath
{
private:
    QVector<QPointF> path;
    QRectF outer = QRectF();
    ColoredDrawTool const tool;
    qreal size;
    qreal eraser_size = 10;
    quint32 hash = 0;

public:
    /// Created new empty path.
    DrawPath(ColoredDrawTool const& tool, QPointF const& start, qreal const size, qreal const eraser_size = 10);
    /// Create new path with given points.
    DrawPath(ColoredDrawTool const& tool, QPointF const* const points, int const number, qreal const size, qreal const eraser_size = 10);
    /// Deprecated: used in legacy file loading function.
    DrawPath(ColoredDrawTool const& tool, QVector<float> const& vec, int const xshift, int const yshift, int const width, int const height, qreal const size, qreal const eraser_size);
    /// Read path from string list. Used in file loading function.
    DrawPath(ColoredDrawTool const& tool, QStringList const& stringList, QPoint const shift, qreal const scale, qreal const size, qreal const eraser_size);
    DrawPath(DrawPath const& old, QPointF const shift, double const scale);
    DrawPath(DrawPath const& old);

    DrawPath operator=(DrawPath const& old) {path.clear(); return DrawPath(old);}
    DrawPath operator=(DrawPath const&& old) {path.clear(); return DrawPath(old);}

    ~DrawPath() {path.clear();}

    /// Called when drawing ends: makes sure that a path contains at least two points such that it can be drawn.
    void endDrawing();
    /// Deprecated: convert path in vector of floates for legacy file saving function.
    void toIntVector(QVector<float>& vec, int const xshift, int const yshift, int const width, int const height) const;
    /// Export path to list of strings representing numbers.
    /// The list contains (alternately) x and y coordinates in point (=inch/72).
    void toText(QStringList& stringList, QPoint const shift, qreal const scale) const;
    void transform(QPointF const& shift, double const scale);
    bool update(DrawPath const& new_path, QPointF const shift, double const scale);
    void updateHash();

    void setEraserSize(qreal const size);

    quint32 getHash() const {return hash;}
    bool isEmpty() const {return path.isEmpty();}
    /// Number of nodes in path.
    int number() const {return path.length();}
    DrawTool getTool() const {return tool.tool;}
    QColor const& getColor() const {return tool.color;}
    qreal getSize() const {return size;}
    QPointF const* data() const {return path.constData();}
    /// Rectangle containing all nodes plus a distance of the eraser size.
    QRectF const& getOuter() const {return outer;}
    /// Rectangle containing all nodes plus a distance of the stroke width.
    QRectF const getOuterDrawing() const;
    /// Return all nodes in this path which are nearer to the given point than eraser_size.
    QVector<int> intersects(QPointF const& point) const;

    /// Append a new node to the path.
    void append(QPointF const& point);
    /// Extract all nodes from index start to index end as a separate path.
    DrawPath* split(int start, int end);
};

#endif // DRAWPATH_H
