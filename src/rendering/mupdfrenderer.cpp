#include "src/rendering/mupdfrenderer.h"

MuPdfRenderer::MuPdfRenderer(const MuPdfDocument *doc) :
    doc(doc)
{
    context = fz_clone_context(doc->getContext());
}

MuPdfRenderer::~MuPdfRenderer()
{
    fz_drop_context(context);
}

const QPixmap MuPdfRenderer::renderPixmap(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return QPixmap();
    const fz_matrix matrix = fz_scale(resolution, resolution);
    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc->getDocument(), page, matrix, fz_device_rgb(context), 0);
    fz_catch(context)
    {
        qWarning() << "MuPDF cannot render page:" << fz_caught_message(context);
        fz_drop_pixmap(context, pixmap);
        return QPixmap();
    }

    // Assume that the pixmap is in RGB colorspace.
    // Write the pixmap in PNM format to a buffer using MuPDF tools.
    fz_buffer *buffer;
    fz_try(context)
        buffer = fz_new_buffer(context, pixmap->stride * pixmap->y + 16);
    fz_catch(context)
    {
        qWarning() << "Failed to allocate memory in Fitz buffer:" << fz_caught_message(context);
        fz_drop_pixmap(context, pixmap);
        fz_clear_buffer(context, buffer);
        fz_drop_buffer(context, buffer);
        return QPixmap();
    }
    fz_output *out = fz_new_output_with_buffer(context, buffer);
    fz_try(context)
        fz_write_pixmap_as_pnm(context, out, pixmap);
    fz_catch(context)
    {
        qWarning() << "Failed to write PNM image to buffer:" << fz_caught_message(context);
        fz_clear_buffer(context, buffer);
        fz_drop_buffer(context, buffer);
        fz_drop_output(context, out);
        fz_drop_pixmap(context, pixmap);
        return QPixmap();
    }
    fz_drop_pixmap(context, pixmap);

    // Load the pixmap from buffer in Qt.
    QPixmap qpixmap;
    if (!qpixmap.loadFromData(buffer->data, buffer->len, "PNM"))
    {
        qWarning() << "Failed to load PNM image from buffer";
    }
    fz_clear_buffer(context, buffer);
    fz_drop_buffer(context, buffer);
    return qpixmap;
}

const PngPixmap * MuPdfRenderer::renderPng(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return nullptr;
    const fz_matrix matrix = fz_scale(resolution, resolution);
    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc->getDocument(), page, matrix, fz_device_rgb(context), 0);
    fz_catch(context)
    {
        qWarning() << "MuPDF cannot render page:" << fz_caught_message(context);
        fz_drop_pixmap(context, pixmap);
        return nullptr;
    }

    // Save the pixmap to buffer in PNG format.
    fz_buffer *buffer;
    fz_try(context)
        buffer = fz_new_buffer_from_pixmap_as_png(context, pixmap, fz_default_color_params);
    fz_catch(context)
    {
        qWarning() << "Fitz failed to write pixmap to PNG buffer:" << fz_caught_message(context);
        fz_clear_buffer(context, buffer);
        fz_drop_buffer(context, buffer);
        return nullptr;
    }
    // Convert the buffer data to QByteArray.
    const QByteArray * data = new QByteArray(reinterpret_cast<const char*>(buffer->data), buffer->len);
    fz_clear_buffer(context, buffer);
    return new PngPixmap(data, page, resolution);
}

bool MuPdfRenderer::isValid() const
{
    return (context != nullptr) && (doc != nullptr);
}
