// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PNGPIXMAP_H
#define PNGPIXMAP_H

#include <QByteArray>
#include "src/config.h"

class QPixmap;

/**
 * @brief PNG-compressed QPixmap image.
 */
class PngPixmap
{
    /// PNG-compressed image.
    const QByteArray* data;

    /// Resolution with which the image was or should be rendered
    /// (in pixels per point, dpi/72).
    const float resolution;

    /// Page number
    const int page;

public:
    /// Constructor: initialize page and resolution; data=NULL.
    PngPixmap(const int page, const float resolution) noexcept;

    /// Constructor: compresses pixmap to PNG. data is null if compression
    /// fails.
    PngPixmap(const QPixmap pixmap, const int page, const float resolution);

    /// Constructor: takes ownership of data.
    PngPixmap(const QByteArray *data, const int page = 0, const float resolution = -1.) noexcept;

    /// Destructor: deletes data.
    ~PngPixmap() noexcept {delete data;}

    /// Decompress the image and return the QPixmap.
    /// The caller takes ownership of the returned QPixmap.
    const QPixmap pixmap() const;

    /// Size of data in bytes.
    int size() const noexcept {return data->size();}

    /// Resolution of the image in pixels per point (dpi/72).
    qreal getResolution() const noexcept {return resolution;}

    /// Page number.
    int getPage() const noexcept {return page;}

    /// Check whether data == NULL
    bool isNull() const noexcept {return data == NULL;}

    /// Return data and set data = NULL
    QByteArray const* takeData();
};

#endif // PNGPIXMAP_H
