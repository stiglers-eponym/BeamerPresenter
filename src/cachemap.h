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

#ifndef CACHEMAP_H
#define CACHEMAP_H

#include <QMap>
#include "basicrenderer.h"

/// QObject rendering pdf pages to images and storing these in a compressed cache.
/// This class handles the complete rendering, owns the cached pages, and owns the
/// cache thread which is used to render pages without affecting the main thread.
class CacheMap : public BasicRenderer
{
    Q_OBJECT

public:
    /// Constructor
    explicit CacheMap(PdfDoc const* doc, PagePart const part = FullPage, QObject* parent = nullptr): BasicRenderer(doc, part, parent), data() {}
    /// Destructor
    ~CacheMap();

    // Get images from cache.
    /// Get an image from cache if available or an empty pixmap otherwise.
    QPixmap const getCachedPixmap(int const page) const;
    /// Get an image from cache or render a new image and save it to cache.
    QPixmap const getPixmap(int const page);
    /// Calculate and return cache ssize in bytes.
    qint64 getSizeBytes() const;
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
    /// Change resolution. This clears cache if the resolution actually changes.
    void changeResolution(double const res) override;

    /// Update cache. This will start cacheThread.
    bool updateCache(int const page);

public slots:
    /// Get cached pages from cacheThread. Called when cacheThread finishes.
    void receiveBytes() override;

private:
    /// Cached slides as png images.
    QMap<int, QByteArray const*> data;

signals:
    /// Notify about changes in cache size (in bytes).
    void cacheSizeChanged(qint64 const size);
};

#endif // CACHEMAP_H
