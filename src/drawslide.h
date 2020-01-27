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

#include <QApplication>
#include <QDataStream>
#include "mediaslide.h"
#include "drawpath.h"
#include "singlerenderer.h"
#include "pathoverlay.h"

class DrawSlide : public MediaSlide
{
    Q_OBJECT
    friend class PathOverlay;

public:
    explicit DrawSlide(QWidget* parent=nullptr);
    explicit DrawSlide(PdfDoc const*const document, int const pageNumber, PagePart const part, QWidget* parent=nullptr);
    ~DrawSlide() override;
    PathOverlay* getPathOverlay() {return pathOverlay;}
    QMap<QString, QList<DrawPath*>> const& getPaths() const {return pathOverlay->paths;}
    double const& getResolution() const {return resolution;}
    virtual bool isShowingTransition() const {return false;}

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void animate(int const oldPageIndex = -1) override;
    virtual void repaintIfPresentation() {update();}
    PathOverlay* pathOverlay;
};

#endif // DRAWSLIDE_H
