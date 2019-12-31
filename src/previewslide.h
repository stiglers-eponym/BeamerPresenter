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

#ifndef PREVIEWSLIDE_H
#define PREVIEWSLIDE_H

#include <QWidget>
#include <QPainter>
#include <QDesktopServices>
#include <QBuffer>
#include <QMouseEvent>
#include "enumerates.h"
#include "pdfdoc.h"
#include "cachemap.h"

class PreviewSlide : public QWidget
{
    Q_OBJECT
public:
    // Constructors and destructor.
    explicit PreviewSlide(QWidget* parent = nullptr) : QWidget(parent), cache(nullptr) {}
    explicit PreviewSlide(PdfDoc const * const document, int const pageNumber, PagePart const part, QWidget* parent=nullptr);
    ~PreviewSlide() override;

    // Rendering and cache.
    virtual void renderPage(int pageNumber);
    int getCacheNumber() const;
    qint64 getCacheSize() const;
    CacheMap* getCacheMap() {return cache;}
    QPixmap const getPixmap(int const page);
    void overwriteCacheMap(CacheMap* newCache = nullptr) {cache = newCache;}

    // Set configuration.
    void setUrlSplitCharacter(QString const& splitCharacter) {urlSplitCharacter=splitCharacter;}
    /// Set pdf document;
    void setDoc(PdfDoc const*const document, PagePart const part) {doc=document; pagePart=part;}

    /// Get current page number
    int pageNumber() const {return pageIndex;}
    /// Get current pdf page.
    Poppler::Page* getPage() {return page;}
    /// Get position of the slide inside the widget (x direction).
    qint16 const& getXshift() const {return shiftx;}
    /// Get position of the slide inside the widget (y direction).
    qint16 const& getYshift() const {return shifty;}

    /// Clear all arrays owned by this.
    virtual void clearAll();

protected:
    PdfDoc const* doc = nullptr;
    Poppler::Page* page = nullptr;
    CacheMap* cache;

    /// Which part of the page is shown on this label.
    PagePart pagePart = FullPage;
    qint16 shiftx = 0;
    qint16 shifty = 0;
    QPixmap pixmap;
    /// resolution in pixels per point = dpi/72
    double resolution = -1.;
    /// page number (starting from 0).
    int pageIndex = 0;
    QList<Poppler::Link*> links;
    QList<QRect> linkPositions;
    QSize oldSize;
    QString urlSplitCharacter = "";
    //char useCache = 1; // 0=don't cache; 1=use cache with internal renderer; 2=use cache with external renderer

    // Handle events.
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    // Rendering. TODO: rewrite.
    QPair<double,double> basicRenderPage(int const pageNumber);

    /// Paint widget on the screen.
    virtual void paintEvent(QPaintEvent*) override;

signals:
    void sendNewPageNumber(int const pageNumber);
    void sendCloseSignal();
    void focusPageNumberEdit();
    void sendShowFullscreen();
    void sendEndFullscreen();
};

#endif // PREVIEWSLIDE_H
