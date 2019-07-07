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

#include <QWidget>
#include <QPainter>
#include <QDesktopServices>
#include <poppler-qt5.h>
#include <QDebug>
#include "enumerates.h"

class BasicSlide : public QWidget
{
    Q_OBJECT

public:
    explicit BasicSlide(QWidget* parent = nullptr) : QWidget(parent) {}
    explicit BasicSlide(Poppler::Page* page, QWidget* parent=nullptr) : QWidget(parent) {renderPage(page);}
    ~BasicSlide() override {}
    virtual void renderPage(Poppler::Page* page, QPixmap const* pixmap=nullptr);
    void setPagePart(PagePart const state) {pagePart=state;}
    int pageNumber() const {return pageIndex;}
    Poppler::Page* getPage() {return page;}
    virtual void clearAll() {page=nullptr;}

protected:
    virtual void paintEvent(QPaintEvent*) override;
    Poppler::Page* page = nullptr;
    PagePart pagePart = FullPage; // Which part of the page is shown on this label
    int shiftx;
    int shifty;
    QPixmap pixmap;
    double resolution; // resolution in pixels per point = dpi/72
    int pageIndex = 0; // page number
    bool pointer_visible = true;

signals:
    void pageNumberChanged(int const pageNumber);
    void sendNewPageNumber(int const pageNumber);
    void sendCloseSignal();
    void focusPageNumberEdit();
    void sendShowFullscreen();
    void sendEndFullscreen();

public slots:
    virtual void togglePointerVisibility();
};

#endif // BASICSLIDE_H
