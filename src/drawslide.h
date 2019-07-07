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

#ifndef DRAWSLIDE_H
#define DRAWSLIDE_H

#include "mediaslide.h"
#include "drawpath.h"

// TODO: move pen strokes to different layer (extra widget) above multimedia widgets

class DrawSlide : public MediaSlide
{
public:
    explicit DrawSlide(QWidget* parent=nullptr) : MediaSlide(parent) {}
    explicit DrawSlide(Poppler::Page* page, QWidget* parent=nullptr) : MediaSlide(page, parent) {}
    ~DrawSlide() override {clearAll();}
    void setTool(DrawTool const newtool);
    void clearPageAnnotations();
    void clearAllAnnotations();
    void clearCache() override;
    void setSize(DrawTool const tool, int const size);

protected:
    void drawAnnotations(QPainter& painter);
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void erase(QPointF const& point);
    DrawTool tool = None;
    QMap<QString, QMap<DrawTool, QList<DrawPath>>> paths;
    QPointF pointerPosition = QPointF();
    QImage enlargedPage;
    int enlargedPageNumber;
    QMap<DrawTool, int> sizes = {{Magnifier,120}, {Torch,80}, {Pointer,10}, {Highlighter,30}, {RedPen,3}, {GreenPen,3}, {Eraser,10}};


public slots:
    void togglePointerVisibility() override;
};

#endif // DRAWSLIDE_H
