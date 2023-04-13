// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QByteArray>
#include <QPixmap>
#include "src/rendering/mupdfdocument.h"
#include "src/rendering/mupdfrenderer.h"
#include "src/rendering/pngpixmap.h"
#include "src/log.h"

#ifndef FZ_VERSION_MAJOR
#define FZ_VERSION_MAJOR 0
#endif
#ifndef FZ_VERSION_MINOR
#define FZ_VERSION_MINOR 0
#endif

fz_pixmap *MuPdfRenderer::renderFzPixmap(const int page, const qreal resolution, fz_context *&ctx) const
{
    if (resolution < 1e-9 || resolution > 1e9 || page < 0)
        return nullptr;

    // Let the main thread prepare everything.
    fz_rect bbox;
    fz_display_list *list = NULL;
    doc->prepareRendering(&ctx, &bbox, &list, page, resolution);

    // If page is not valid (too large), the NULLs will be unchanged.
    if (ctx == nullptr || list == nullptr)
        return nullptr;

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
    fz_device *dev = nullptr;
    fz_pixmap *pixmap = nullptr;
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
        fz_run_display_list(ctx, list, dev, fz_identity, bbox, nullptr);
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
        qWarning() << "Fitz failed to create or render pixmap:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_drop_context(ctx);
        return nullptr;
    }
    return pixmap;
}

const QPixmap MuPdfRenderer::renderPixmap(const int page, const qreal resolution) const
{
    QPixmap qpixmap;
    fz_context *ctx = nullptr;
    fz_pixmap *pixmap = renderFzPixmap(page, resolution, ctx);
    if (!pixmap || !ctx)
        return qpixmap;

    // Assume that the pixmap is in RGB colorspace.
    // Write the pixmap in PNM format to a buffer using MuPDF tools.
    fz_buffer *buffer = nullptr;
    fz_output *out = nullptr;
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
        qWarning() << "Fitz failed to write PNM image to buffer:" << fz_caught_message(ctx);
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
        return nullptr;

    // Save the pixmap to buffer in PNG format.
    fz_buffer *buffer = nullptr;
    fz_try(ctx)
        // Here valgrind complained about "Use of uninitialised value of size 8"
        buffer = fz_new_buffer_from_pixmap_as_png(ctx, pixmap, fz_default_color_params);
    fz_always(ctx)
        fz_drop_pixmap(ctx, pixmap);
    fz_catch(ctx)
    {
        qWarning() << "Fitz falied to allocate buffer:" << fz_caught_message(ctx);
        fz_drop_buffer(ctx, buffer);
        fz_drop_context(ctx);
        return nullptr;
    }

    // Convert the buffer data to QByteArray.
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 17)
    const QByteArray *data = buffer ? new QByteArray(reinterpret_cast<const char*>(buffer->data), buffer->len) : nullptr;
#else
    const QByteArray *data = nullptr;
    if (buffer)
    {
        unsigned char *buffer_data;
        const int buffer_size = fz_buffer_storage(ctx, buffer, &buffer_data);
        data = new QByteArray(reinterpret_cast<const char*>(buffer_data), buffer_size);
    }
#endif
    fz_drop_buffer(ctx, buffer);
    fz_drop_context(ctx);
    return new PngPixmap(data, page, resolution);
}
