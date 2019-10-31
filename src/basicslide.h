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

#ifndef BASICSLIDE_H
#define BASICSLIDE_H

#include <QPainter>
#include <QDesktopServices>
#include "enumerates.h"
#include "pdfdoc.h"

class BasicSlide : public QWidget
{
    Q_OBJECT

public:
    explicit BasicSlide(QWidget* parent = nullptr) : QWidget(parent) {}
    explicit BasicSlide(PdfDoc const*const document, int const pageNumber, QWidget* parent=nullptr);
    ~BasicSlide() override {}
    virtual void renderPage(int const pageNumber, QPixmap const* pix=nullptr);
    void setPagePart(PagePart const state) {pagePart=state;}
    int pageNumber() const {return pageIndex;}
    virtual void clearAll() {page=nullptr;}
    void setDoc(PdfDoc const*const document) {doc=document;}
    Poppler::Page* getPage() {return page;}
    quint16 const& getXshift() const {return shiftx;}
    quint16 const& getYshift() const {return shifty;}

protected:
    virtual void paintEvent(QPaintEvent*) override;
    Poppler::Page* page = nullptr;
    PagePart pagePart = FullPage; // Which part of the page is shown on this label
    quint16 shiftx = 0;
    quint16 shifty = 0;
    QPixmap pixmap;
    double resolution = -1.; // resolution in pixels per point = dpi/72
    int pageIndex = 0; // page number
    PdfDoc const * doc = nullptr;

signals:
    void sendNewPageNumber(int const pageNumber);
    void sendCloseSignal();
    void focusPageNumberEdit();
    void sendShowFullscreen();
    void sendEndFullscreen();
};

#endif // BASICSLIDE_H
