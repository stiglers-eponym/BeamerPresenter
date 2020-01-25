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

#ifndef CACHETHREAD_H
#define CACHETHREAD_H

#include <QObject>
#include <QThread>
#include <QPixmap>
#include "externalrenderer.h"

class BasicRenderer;

/// QThread used for rendering slides to png images in cache.
/// This thread is owned by CacheMap objects, but rendering a page to cache
/// using this thread is always triggered by ControlScreen::updateCacheStep.
class CacheThread : public QThread
{
    Q_OBJECT

private:
    /// Next page which will be rendered. This number is set via setPage(int) by CacheMap.
    int newPage = 0;
    /// Currently rendered page. This page is only adapted to newPage at the beginning of run().
    int page = 0;
    /// CacheMap object owning this.
    BasicRenderer const* master;
    /// Cached page as a png image. This is accessible by CacheMap as the result of run().
    QByteArray const* bytes = nullptr;

public:
    /// Constructor.
    CacheThread(BasicRenderer const* cache, QObject* parent = nullptr) : QThread(parent), master(cache) {}
    /// Set page which should be rendered next.
    void setPage(int const pageNumber) {newPage = pageNumber;}
    /// This must be called exactly once after run() finished.
    QByteArray const* getBytes();
    /// Get page which this is currently rendering.
    int getPage() const {return page;}
    /// Do the work: Set page=newPage, render it, and save the compressed page in bytes.
    void run() override;
};

#endif // CACHETHREAD_H
