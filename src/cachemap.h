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
#include "pdfdoc.h"

class CacheMap : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    CacheMap(PdfDoc const* doc, QObject* parent = nullptr) : QObject(parent) {pdf=doc;}
    /// Destructor
    ~CacheMap();

    // Get images from cache.
    /// Get an image from cache if available or an empty pixmap otherwise.
    QPixmap const getCachedPixmap(int const page) const;
    /// Get an image from cache or render a new image and save it to cache.
    QPixmap const getPixmap(int const page);
    /// Get an image from cache or render a new image.
    QPixmap const renderPixmap(int const page) const;
    /// Get cache size in bytes.
    qint64 getSizeBytes() const {return sizeBytes;}
    /// Set data from pixmap.
    qint64 setPixmap(int const page, QPixmap const* pix);

    // Settings.
    void setPagePart(PagePart const part) {pagePart = part;}
    /// Change resolution. This clears cache and changes the resolution.
    void changeResolution(double const res);
    /// Set custom renderer. When only empty strings are given, the renderer is set to popper (internal).
    void setRenderer(QString const& renderCommand = "", QString const& fileName = "");
    /// Get renderer command.
    QString const getRenderCommand(int const page) const;
    /// Get page part.
    PagePart getPagePart() const {return pagePart;}

private:
    /// Cached slides as png images.
    QMap<int, QByteArray const*> data;
    /// Cache size in bytes.
    qint64 sizeBytes = 0;
    /// PDF document.
    PdfDoc const* pdf;
    /// Resolution of the pixmap.
    double resolution = 0.;
    /// Part of the page showing the relevant part for this cache.
    PagePart pagePart = FullPage;
    /// Command for external renderer.
    QString renderer = "";
};

#endif // CACHEMAP_H
