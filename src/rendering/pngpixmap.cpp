#include "src/rendering/pngpixmap.h"

PngPixmap::PngPixmap(const int page, const float resolution) :
     data(nullptr),
     resolution(resolution),
     page(page)
{};

PngPixmap::PngPixmap(const QByteArray *data, const int page, const float resolution) :
     data(data),
     resolution(resolution),
     page(page)
{};

PngPixmap::PngPixmap(const QPixmap pixmap, const int page, const float resolution) :
    data(nullptr),
    resolution(resolution),
    page(page)
{
    // Check if the given pixmap is nontrivial
    if (pixmap.isNull())
        return;

    // Save the pixmap as PNG image.
    // First create a writable QByteArray and a QBuffer to write to it.
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    // Save the pixmap as png to the buffer.
    if (!pixmap.save(&buffer, "PNG"))
    {
        qWarning() << "Compressing image to PNG failed";
        delete bytes;
        return;
    }

    // Keep the result in data.
    data = bytes;
}

const QPixmap PngPixmap::pixmap() const
{
    QPixmap pixmap;
    if (data == nullptr || !pixmap.loadFromData(*data, "PNG"))
        qWarning() << "Loading image from PNG failed";
    return pixmap;
}

const QByteArray * PngPixmap::takeData()
{
    const QByteArray * const pointer = data;
    data = nullptr;
    return pointer;
}
