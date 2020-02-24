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

#ifndef SINGLERENDERER_H
#define SINGLERENDERER_H

#include "basicrenderer.h"

/// Simplest class for handling a CacheThread.
/// This class is used to control a CacheThread for rendering in parallel to the main thread, but without cache management.
/// Only the currently rendered page is stored in this object.
class SingleRenderer : public BasicRenderer
{
    Q_OBJECT

public:
    /// Constructor
    explicit SingleRenderer(PdfDoc const* doc, PagePart const part = FullPage, QObject* parent = nullptr): BasicRenderer(doc, part, parent) {}
    /// Destructor
    ~SingleRenderer();

    /// Get the cached image.
    QPixmap const getPixmap();
    /// Update cache. This will start cacheThread.
    void renderPage(int const page);
    bool resultReady() const {return data != nullptr;}
    int getPage() const {return page;}

public slots:
    /// Get cached pages from cacheThread. Called when cacheThread finishes.
    void receiveBytes() override;

private:
    QByteArray const* data = nullptr;
    int page = -1;
};

#endif // SINGLERENDERER_H
