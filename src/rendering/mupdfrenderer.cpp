#include <QDebug>
#include <string>
#include "src/rendering/mupdfrenderer.h"
#include "src/rendering/pngpixmap.h"
#include "src/preferences.h"

fz_pixmap *MuPdfRenderer::renderFzPixmap(const int page, const qreal resolution, fz_context *&ctx) const
{
    if (resolution < 1e-9 || resolution > 1e9 || page < 0)
        return NULL;

    // Let the main thread prepare everything.
    fz_rect bbox;
    fz_display_list *list = NULL;
    doc->prepareRendering(&ctx, &bbox, &list, page, resolution);

    // If page is not valid (too large), the NULLs will be unchanged.
    if (ctx == NULL || list == NULL)
        return NULL;

    // Adapt bbox to page part.
    switch (page_part)
    {
    case LeftHalf:
        bbox.x1 /= 2;
        break;
    case RightHalf:
        bbox.x0 = (bbox.x0 + bbox.x1)/2;
        break;
    default:
        break;
    }

    // Create a local clone of the main thread's context.
    ctx = fz_clone_context(ctx);

    // Create pixmap and render page to it.
    fz_device *dev = NULL;
    fz_pixmap *pixmap;
    fz_var(pixmap);
    fz_var(dev);
    fz_try(ctx)
    {
        // Create the pixmap and fill it with white background.
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR > 12)
        pixmap = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(bbox), NULL, 0);
#else
        pixmap = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_irect_from_rect(bbox), NULL, 0);
#endif
        fz_clear_pixmap_with_value(ctx, pixmap, 0xff);
        // Create a device for rendering the given display list to pixmap.
        dev = fz_new_draw_device(ctx, fz_identity, pixmap);
        // Do the main work: Render the display list to pixmap.
        fz_run_display_list(ctx, list, dev, fz_identity, bbox, NULL);
    }
    fz_always(ctx)
    {
        if (dev)
        {
            fz_close_device(ctx, dev);
            fz_drop_device(ctx, dev);
        }
        fz_drop_display_list(ctx, list);
    }
    fz_catch(ctx)
    {
        warn_msg << "Fitz failed to create or render pixmap:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_drop_context(ctx);
        return NULL;
    }
    return pixmap;
}

const QPixmap MuPdfRenderer::renderPixmap(const int page, const qreal resolution) const
{
    QPixmap qpixmap;
    fz_context *ctx = NULL;
    fz_pixmap *pixmap = renderFzPixmap(page, resolution, ctx);
    if (!pixmap || !ctx)
        return qpixmap;

    // Assume that the pixmap is in RGB colorspace.
    // Write the pixmap in PNM format to a buffer using MuPDF tools.
    fz_buffer *buffer = NULL;
    fz_output *out = NULL;
    fz_var(pixmap);
    fz_var(buffer);
    fz_var(out);
    fz_try(ctx)
    {
        buffer = fz_new_buffer(ctx, pixmap->stride * pixmap->y + 16);
        out = fz_new_output_with_buffer(ctx, buffer);
        fz_write_pixmap_as_pnm(ctx, out, pixmap);
    }
    fz_always(ctx)
    {
        fz_drop_output(ctx, out);
        fz_drop_pixmap(ctx, pixmap);
    }
    fz_catch(ctx)
    {
        warn_msg << "Fitz failed to write PNM image to buffer:" << fz_caught_message(ctx);
        fz_drop_buffer(ctx, buffer);
        fz_drop_context(ctx);
        return qpixmap;
    }

    // Load the pixmap from buffer in Qt.
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 17)
    if (!buffer || !qpixmap.loadFromData(buffer->data, buffer->len, "PNM"))
        qWarning() << "Failed to load PNM image from buffer";
#else
    if (buffer)
    {
        unsigned char *buffer_data;
        const int buffer_size = fz_buffer_storage(ctx, buffer, &buffer_data);
        qpixmap.loadFromData(buffer_data, buffer_size, "PNM");
    }
#endif
    fz_drop_buffer(ctx, buffer);
    fz_drop_context(ctx);
    return qpixmap;
}

const PngPixmap * MuPdfRenderer::renderPng(const int page, const qreal resolution) const
{
    fz_context *ctx;
    fz_pixmap *pixmap = renderFzPixmap(page, resolution, ctx);
    if (!pixmap || !ctx)
        return NULL;

    // Save the pixmap to buffer in PNG format.
    fz_buffer *buffer = NULL;
    fz_var(pixmap);
    fz_var(buffer);
    fz_try(ctx)
        // Here valgrind complained about "Use of uninitialised value of size 8"
        buffer = fz_new_buffer_from_pixmap_as_png(ctx, pixmap, fz_default_color_params);
    fz_always(ctx)
        fz_drop_pixmap(ctx, pixmap);
    fz_catch(ctx)
    {
        warn_msg << "Fitz falied to allocate buffer:" << fz_caught_message(ctx);
        fz_drop_buffer(ctx, buffer);
        fz_drop_context(ctx);
        return NULL;
    }

    // Convert the buffer data to QByteArray.
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 17)
    const QByteArray *data = buffer ? new QByteArray(reinterpret_cast<const char*>(buffer->data), buffer->len) : NULL;
#else
    const QByteArray *data;
    if (buffer)
    {
        unsigned char *buffer_data;
        const int buffer_size = fz_buffer_storage(ctx, buffer, &buffer_data);
        data = new QByteArray(reinterpret_cast<const char*>(buffer_data), buffer_size);
    }
    else
        data = NULL;
#endif
    fz_drop_buffer(ctx, buffer);
    fz_drop_context(ctx);
    return new PngPixmap(data, page, resolution);
}
