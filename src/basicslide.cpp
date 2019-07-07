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

#include "basicslide.h"

void BasicSlide::renderPage(Poppler::Page* page, QPixmap const* pix)
{
    if (page == nullptr)
        return;

    // Set the new page and basic properties
    this->page = page;
    pageIndex = page->index();
    QSizeF pageSize = page->pageSizeF();
    // This is given in point = inch/72 ≈ 0.353mm (Did they choose these units to bother programmers?)

    // Place the page as an image of the correct size at the correct position
    // The lower left corner of the image will be located at (shiftx, shifty)
    double pageHeight=pageSize.height(), pageWidth=pageSize.width();
    // The page image must be split if the beamer option "notes on second screen" is set.
    if (pagePart != FullPage)
        pageWidth /= 2;
    // Check it width or height is the limiting constraint for the size of the displayed slide and calculate the resolution
    // resolution is calculated in pixels per point = dpi/72.
    if (width() * pageHeight > height() * pageWidth) {
        // the width of the label is larger than required
        resolution = double(height()) / pageHeight;
        shiftx = int(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        // the height of the label is larger than required
        resolution = double(width()) / pageWidth;
        shifty = int(height()/2 - resolution/2 * pageHeight);
        shiftx = 0;
    }

    // Adjustments if only parts of the page are shown:
    // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
    if (pagePart == RightHalf)
        shiftx -= width();

    // Get the image
    if (pix != nullptr) {
        // A pixmap was passed to this function. Display this pixmap as the page image.
        if (pagePart != FullPage) {
            // The pixmap might show both notes and presentation.
            // Check the width to decide whether the image shows only the relevant part or the full page.
            if (pix->width() > 1.5*pixmap.width()) {
                // Assume that the pixmap shows notes and presentation.
                if (pagePart == LeftHalf)
                    pixmap = pix->copy(0, 0, pix->width()/2, pix->height());
                else
                    pixmap = pix->copy(pix->width()/2, 0, pix->width()/2, pix->height());
            }
            else
                pixmap = *pix;
        }
        else
            pixmap = *pix;
    }
    else {
        if (pagePart == FullPage)
            pixmap = QPixmap::fromImage(page->renderToImage(72*resolution, 72*resolution));
        else {
            QImage image = page->renderToImage(72*resolution, 72*resolution);
            if (pagePart == LeftHalf)
                pixmap = QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
            else
                pixmap = QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
        }
    }

    // Show the page on the screen.
    // One could show the page in any case to make it slightly more responsive, but this can lead to a short interruption by a different image.
    // All operations before the next call to update() are usually very fast.
    update();
    emit pageNumberChanged(pageIndex);
}

void BasicSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(shiftx, shifty, pixmap);
}

void BasicSlide::togglePointerVisibility()
{
    if (pointer_visible) {
        pointer_visible = false;
        setMouseTracking(false);
        setCursor(Qt::BlankCursor);
    }
    else {
        pointer_visible = true;
        setMouseTracking(true);
        setCursor(Qt::ArrowCursor);
    }
}
