#ifndef PNGPIXMAP_H
#define PNGPIXMAP_H

#include <QByteArray>
#include <QPixmap>
#include <QBuffer>
#include <QDebug>

/// PNG-compressed QPixmap image.
class PngPixmap
{
    /// PNG-compressed image.
    const QByteArray* data;
    /// Resolution with which the image was or should be rendered.
    const float resolution;
    /// Page number
    const int page;

public:
    PngPixmap(const int page, const float resolution);
    PngPixmap(const QPixmap pixmap, const int page, const float resolution);
    PngPixmap(const QByteArray *data, const int page = 0, const float resolution = -1.);
    ~PngPixmap() {delete data;}

    /// Uncompress the image and return the QPixmap.
    /// The caller takes ownership of the returned QPixmap.
    const QPixmap pixmap() const;

    /// Size in byte of data.
    int size() const {return data->size();}

    /// resolution of the image
    qreal getResolution() const {return resolution;}

    /// Page number
    int getPage() const {return page;}

    /// Check whether data == nullptr
    bool isNull() const {return data == nullptr;}

    /// Return data and set data = nullptr
    QByteArray const* takeData();
};

#endif // PNGPIXMAP_H
