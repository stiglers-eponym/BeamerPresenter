#include "src/rendering/pngpixmap.h"
#include <QDebug>
#include <QPixmap>
#include <QBuffer>

PngPixmap::PngPixmap(const int page, const float resolution) noexcept :
     data(NULL),
     resolution(resolution),
     page(page)
{};

PngPixmap::PngPixmap(const QByteArray *data, const int page, const float resolution) noexcept :
     data(data),
     resolution(resolution),
     page(page)
{};

PngPixmap::PngPixmap(const QPixmap pixmap, const int page, const float resolution) :
    data(NULL),
    resolution(resolution),
    page(page)
{
    // Check if the given pixmap is nontrivial
    if (pixmap.isNull() || pixmap.size().isEmpty() || pixmap.isDetached())
        return;

    // Save the pixmap as PNG image.
    // First create a writable QByteArray and a QBuffer to write to it.
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    // Save the pixmap as png to the buffer.
    const bool success = buffer.open(QIODevice::WriteOnly) && pixmap.save(&buffer, "PNG");
    buffer.close();
    if (success)
    {
        // Keep the result in data.
        data = bytes;
    }
    else
    {
        // saving failed, delete result.
        delete bytes;
        qWarning() << "Compressing image to PNG failed";
    }
}

const QPixmap PngPixmap::pixmap() const
{
    QPixmap pixmap;
    if (data == NULL || data->isEmpty() || !pixmap.loadFromData(*data, "PNG"))
        qWarning() << "Loading image from PNG failed";
    return pixmap;
}

const QByteArray * PngPixmap::takeData()
{
    const QByteArray * const pointer = data;
    data = NULL;
    return pointer;
}
