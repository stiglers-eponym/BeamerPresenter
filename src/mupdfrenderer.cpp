#include "mupdfrenderer.h"

MuPdfRenderer::MuPdfRenderer(const QString &filename)
{
    // This code is mainly copied from a MuPDF example file, see mupdf.com

    // Create the Fitz context for MuPDF.
    context = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (context == nullptr)
    {
        qWarning() << "Failed to create Fitz context";
        doc = nullptr;
        return;
    }

    // Try to register default document handlers.
    fz_try(context)
        fz_register_document_handlers(context);
    fz_catch(context)
    {
        qWarning() << "MuPdf cannot register document handlers:" << fz_caught_message(context);
        doc = nullptr;
        fz_drop_context(context);
        context =  nullptr;
        return;
    }

    // Open the document.
    // TODO: check the encoding...
    const char * const name = filename.toUtf8().data();
    fz_try(context)
        doc = fz_open_document(context, name);
    fz_catch(context)
    {
        qWarning() << "MuPdf cannot open document:" << fz_caught_message(context);
        doc = nullptr;
        fz_drop_context(context);
        context =  nullptr;
        return;
    }
    delete name; // TODO: is this correct?
}

MuPdfRenderer::~MuPdfRenderer()
{
    fz_drop_document(context, doc);
    fz_drop_context(context);
}

const QPixmap MuPdfRenderer::renderPixmap(const int page, const qreal resolution) const
{
    const fz_matrix matrix = fz_scale(72.*resolution, 72.*resolution);
    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc, page, matrix, fz_device_rgb(context), 0);
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

const QByteArray * MuPdfRenderer::renderPng(const int page, const qreal resolution) const
{
    const fz_matrix matrix = fz_scale(72.*resolution, 72.*resolution);
    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc, page, matrix, fz_device_rgb(context), 0);
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
    return data;
}
