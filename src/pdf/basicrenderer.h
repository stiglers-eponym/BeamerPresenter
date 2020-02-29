/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2020  stiglers-eponym

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

#ifndef BASICRENDERER_H
#define BASICRENDERER_H

#include <QObject>
#include <QBuffer>
#include <QByteArray>
#include "pdfdoc.h"
#include "cachethread.h"

/// Abstract class for rendering pages using a CacheThread.
/// Classes inheriting from BasicRenderer can be used to render slides in a different thread.
/// These classes are SingleRenderer (rendering and storing a single page), and CacheMap (storing cached pages in a QMap).
class BasicRenderer : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    explicit BasicRenderer(PdfDoc const* doc, PagePart const part = FullPage, QObject* parent = nullptr);
    /// Destructor
    /// cacheThread must be deleted by destructors of child classes.
    ~BasicRenderer() {};
    /// Get cache thread.
    CacheThread* getCacheThread() {return cacheThread;}
    /// Render page using poppler.
    QPixmap const renderPixmap(int const page) const;

    /// Is a cache thread running?
    bool threadRunning() const {return cacheThread->isRunning();}
    qreal getResolution() const {return resolution;}

    // Settings.
    /// Change resolution.
    virtual void changeResolution(double const res) {resolution=res;}
    /// Set custom renderer. When only empty strings are given, the renderer is set to popper (internal).
    void setRenderer(QString const renderer = "") {renderCommand = renderer;}
    /// Get renderer command.
    QString const getRenderCommand(int const page) const;
    /// Get page part.
    PagePart getPagePart() const {return pagePart;}

public slots:
    /// Get cached pages from cacheThread. Called when cacheThread finishes.
    virtual void receiveBytes() = 0;

protected:
    /// PDF document.
    PdfDoc const* const pdf;
    /// Resolution of the pixmap.
    qreal resolution = -1.;
    /// Part of the page showing the relevant part for this cache.
    PagePart const pagePart;
    /// Command for external renderer.
    QString renderCommand = "";
    /// Separate thread used to render pages to compressed cache.
    CacheThread* cacheThread;

signals:
    /// Nofity that cacheThread has finished and this has received a new compressed page from cacheThread.
    void cacheThreadFinished();

};

#endif // BASICRENDERER_H
