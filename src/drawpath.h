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
#include "enumerates.h"

class DrawPath
{
private:
    QVector<QPointF> path;
    QRectF outer = QRectF();
    quint16 eraser_size = 10;
    ColoredDrawTool tool;
    quint32 hash = 0;
public:
    DrawPath(ColoredDrawTool const& tool, QPointF const& start, quint16 const eraser_size = 10);
    DrawPath(ColoredDrawTool const& tool, QPointF* const points, int const number, quint16 const eraser_size = 10);
    DrawPath(DrawPath const& old, QPointF const shift, double const scale);
    DrawPath(DrawPath const& old);
    DrawPath(ColoredDrawTool const& tool, QVector<float> const& vec, int const xshift, int const yshift, int const width, int const height, quint16 const eraser_size);
    ~DrawPath() {path.clear();}
    void toIntVector(QVector<float>& vec, int const xshift, int const yshift, int const width, int const height) const;
    void transform(QPointF const& shift, double const scale);
    void updateHash();
    void setEraserSize(quint16 const size);
    void clear() {path.clear(); outer=QRectF(); hash=0;}
    quint32 getHash() const {return hash;}
    bool isEmpty() const {return path.isEmpty();}
    int number() const {return path.length();}
    DrawTool getTool() const {return tool.tool;}
    QColor const& getColor() const {return tool.color;}
    QPointF const* data() const {return path.constData();}
    QRectF const& getOuter() const {return outer;}
    QVector<int> intersects(QPointF const& point) const;
    void append(QPointF const& point);
    DrawPath* split(int start, int end);
    DrawPath operator=(DrawPath const& old) {path.clear(); return DrawPath(old);}
    DrawPath operator=(DrawPath const&& old) {path.clear(); return DrawPath(old);}
};

#endif // DRAWPATH_H
