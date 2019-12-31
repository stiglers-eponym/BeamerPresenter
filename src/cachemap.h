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

#ifndef CACHEMAP_H
#define CACHEMAP_H

#include <QObject>
#include <QMap>
#include <QBuffer>
#include <QByteArray>
#include <QTimer>
#include "pdfdoc.h"
#include "cachethread.h"

class CacheMap : public QObject
{
    Q_OBJECT

public:
    /// Constructors
    CacheMap(PdfDoc const* doc, PagePart const part, QObject* parent = nullptr) : QObject(parent), data(), pdf(doc), pagePart(part) {setupCacheThread();}
    CacheMap(CacheMap& other);
    /// Destructor
    ~CacheMap();
    void setupCacheThread();

    // Get images from cache.
    /// Get an image from cache if available or an empty pixmap otherwise.
    QPixmap const getCachedPixmap(int const page) const;
    /// Get an image from cache or render a new image and save it to cache.
    QPixmap const getPixmap(int const page);
    /// Get an image from cache or render a new image.
    QPixmap const renderPixmap(int const page) const;
    /// Get cache size in bytes.
    qint64 getSizeBytes() const {return size;}
    /// Set data from pixmap.
    qint64 setPixmap(int const page, QPixmap const* pix);
    /// Clear cache.
    void clearCache();
    /// Is a page contained in cache?
    bool contains(int const page) {return data.contains(page);}
    /// Number of cached slides.
    int length() const {return data.size();}
    /// Delete a page from cache and return its size.
    qint64 clearPage(int const page);

    /// Update cache. This will start cacheTimer.
    bool updateCache(int const page);
    /// Is a cache thread running?
    bool threadRunning() {return cacheThread != nullptr && cacheThread->isRunning();}

    // Settings.
    /// Change resolution. This clears cache if the resolution actually changes.
    void changeResolution(double const res);
    /// Set custom renderer. When only empty strings are given, the renderer is set to popper (internal).
    void setRenderer(QString const renderer = "") {renderCommand = renderer;}
    /// Get renderer command.
    QString const getRenderCommand(int const page) const;
    /// Get page part.
    PagePart getPagePart() const {return pagePart;}

public slots:
    void receiveBytes(int const page, QByteArray const*);

private:
    /// Cached slides as png images.
    QMap<int, QByteArray const*> data;
    /// Cache size in bytes.
    qint64 size = 0;
    /// PDF document.
    PdfDoc const* pdf;
    /// Resolution of the pixmap.
    double resolution = 0.;
    /// Part of the page showing the relevant part for this cache.
    PagePart pagePart = FullPage;
    /// Command for external renderer.
    QString renderCommand = "";

    // These should remain nullptr if cache is disabled.
    CacheThread* cacheThread = nullptr;
    QTimer* cacheTimer = nullptr;

signals:
    void cacheSizeChanged(qint64 const size);
};

#endif // CACHEMAP_H
