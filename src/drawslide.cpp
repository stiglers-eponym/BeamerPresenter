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

#include "drawslide.h"
#include "pathoverlay.h"

DrawSlide::DrawSlide(QWidget* parent) :
    MediaSlide(parent),
    pathOverlay(new PathOverlay(this))
{}

DrawSlide::DrawSlide(PdfDoc const*const document, int const pageNumber, PagePart const part, QWidget* parent) :
    MediaSlide(document, pageNumber, part, parent),
    pathOverlay(new PathOverlay(this))
{}

DrawSlide::~DrawSlide()
{
    delete pathOverlay;
    clearAll();
}

void DrawSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (pagePart == RightHalf)
        painter.drawPixmap(shiftx + width(), shifty, pixmap);
    else
        painter.drawPixmap(shiftx, shifty, pixmap);
    pathOverlay->drawAnnotations(painter);
}

void DrawSlide::resizeEvent(QResizeEvent*)
{
    if (resolution < 0 || page == nullptr)
        return;
    qint16 const oldshiftx = shiftx, oldshifty = shifty;
    double const oldRes = resolution;
    QSizeF pageSize = page->pageSizeF();
    double pageWidth=pageSize.width();
    if (pagePart != FullPage)
        pageWidth /= 2;
    if (width() * pageSize.height() > height() * pageWidth) {
        resolution = double(height()) / pageSize.height();
        shiftx = qint16(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        resolution = double(width()) / pageWidth;
        shifty = qint16(height()/2 - resolution/2 * pageSize.height());
        shiftx = 0;
    }
    pathOverlay->setGeometry(geometry());
    pathOverlay->rescale(oldshiftx, oldshifty, oldRes);
}

void DrawSlide::mouseReleaseEvent(QMouseEvent *event)
{
    if (page == nullptr)
        return;
    switch (event->button())
    {
    case Qt::LeftButton:
    case Qt::MidButton:
        followHyperlinks(event->pos());
        break;
    default:
        break;
    }
    event->accept();
}

void DrawSlide::animate(const int oldPageIndex)
{
     if (oldPageIndex != pageIndex) {
         pathOverlay->resetCache();
     }
}
