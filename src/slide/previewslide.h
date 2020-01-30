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
#include "../enumerates.h"
#include "../pdf/pdfdoc.h"
#include "../pdf/cachemap.h"

/// Basic slide.
/// This widget shows a slide on the screen and links of navigation and action type.
/// For rendering of slides this contains a CacheMap object which renders slides to cache.
class PreviewSlide : public QWidget
{
    Q_OBJECT

public:
    // Constructors and destructor.
    /// Only create an empty widget. Cache is not initialized.
    explicit PreviewSlide(QWidget* parent = nullptr) : QWidget(parent), cache(nullptr) {}
    /// Create a full PreviewSlide, initialize cache and show (render) the given slide number.
    explicit PreviewSlide(PdfDoc const * const document, int const pageNumber, PagePart const part, QWidget* parent=nullptr);
    ~PreviewSlide() override;

    /// Show a page on the widget.
    virtual void renderPage(int pageNumber);
    /// Cache map used to render pages.
    CacheMap* getCacheMap() {return cache;}
    /// Cache map used to render pages.
    CacheMap const* getCacheMap() const {return cache;}
    /// Currently shown slide as pixmap.
    QPixmap const getPixmap(int const page);
    /// Overwrite PreviewSlide::cacheMap without deleting it.
    void overwriteCacheMap(CacheMap* newCache) {cache = newCache;}

    // Set configuration.
    /// Set urlSplitCharacter.
    void setUrlSplitCharacter(QString const& splitCharacter) {urlSplitCharacter=splitCharacter;}
    /// Set pdf document and PagePart.
    void setDoc(PdfDoc const*const document, PagePart const part) {doc=document; pagePart=part;}

    /// Get current page number
    int pageNumber() const {return pageIndex;}
    /// Get current pdf page.
    Poppler::Page const* getPage() {return page;}
    /// Get position of the slide inside the widget (x direction).
    qint16 const& getXshift() const {return shiftx;}
    /// Get position of the slide inside the widget (y direction).
    qint16 const& getYshift() const {return shifty;}

    /// Clear all contents of the label.
    /// This function is called when the document is reloaded or the program is closed and everything should be cleaned up.
    virtual void clearAll();

protected:
    /// PDF document.
    PdfDoc const* doc = nullptr;
    /// Currently displayed slide.
    Poppler::Page const* page = nullptr;
    /// Cache map for fast rendering of pages
    CacheMap* cache;

    /// Defines which part of the page is shown on this label.
    PagePart pagePart = FullPage;
    /// X offset of the side in the current widget.
    qint16 shiftx = 0;
    /// Y offset of the side in the current widget.
    qint16 shifty = 0;
    /// Pixmap of currently displayed slide.
    QPixmap pixmap;
    /// resolution in pixels per point = dpi/72
    double resolution = -1.;
    /// page number (starting from 0).
    int pageIndex = 0;
    /// List of links on the current slide.
    QList<Poppler::Link*> links;
    /// List of positions of links of the current slide.
    QList<QRect> linkPositions;
    /// Size of the widget, saved the last time when a page was rendered.
    /// This is compared to the current size of the widget when a new page is rendered.
    QSize oldSize;
    /// Character used to split links to files into a file path and a list of arguments.
    QString urlSplitCharacter = "";

    // Handle events.
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    /// Function doing the main work in PreviewSlide::renderPage and MediaSlide::renderPage.
    QPair<double,double> basicRenderPage(int const pageNumber);

    /// Paint widget on the screen.
    virtual void paintEvent(QPaintEvent*) override;

signals:
    /// Send a new page number to ControlScreen and PresentationScreen. The new page will be shown.
    void sendNewPageNumber(int const pageNumber);
    /// Quit the application.
    void sendCloseSignal();
    /// Focus on the page number editor.
    void focusPageNumberEdit();
    void sendShowFullscreen();
    void sendEndFullscreen();
};

#endif // PREVIEWSLIDE_H
