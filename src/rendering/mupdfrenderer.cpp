#include "src/rendering/mupdfrenderer.h"

const QPixmap MuPdfRenderer::renderPixmap(const int page, const qreal resolution) const
{
    // Let the main thread prepare everything.
    fz_context *ctx = nullptr;
    fz_rect bbox;
    fz_display_list *list = nullptr;
    doc->prepareRendering(&ctx, &bbox, &list, page, resolution);

    // If page is not valid (too large), the nullptrs will be unchanged.
    if (ctx == nullptr || list == nullptr)
        return QPixmap();

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

    // Create a new pixmap and fill it with background color (white).
    // TODO: check if this causes problems with different PDF background colors.
    fz_pixmap *pixmap = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(bbox), NULL, 0);
    fz_clear_pixmap_with_value(ctx, pixmap, 0xff);

    // Create a device for rendering the given display list to pixmap.
    fz_device *dev = fz_new_draw_device(ctx, fz_identity, pixmap);

    // Do the main work: Render the display list to pixmap.
    fz_try(ctx)
        fz_run_display_list(ctx, list, dev, fz_identity, bbox, NULL);
    fz_catch(ctx)
    {
        qWarning() << "Fitz failed to render pixmap:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_drop_display_list(ctx, list);
        fz_close_device(ctx, dev);
        fz_drop_device(ctx, dev);
        fz_drop_context(ctx);
        return QPixmap();
    }

    // Clean up device and list.
    fz_drop_display_list(ctx, list);
    fz_close_device(ctx, dev);
    fz_drop_device(ctx, dev);

    // Assume that the pixmap is in RGB colorspace.
    // Write the pixmap in PNM format to a buffer using MuPDF tools.
    fz_buffer *buffer = nullptr;
    fz_try(ctx)
        buffer = fz_new_buffer(ctx, pixmap->stride * pixmap->y + 16);
    fz_catch(ctx)
    {
        qWarning() << "Failed to allocate memory in Fitz buffer:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_clear_buffer(ctx, buffer);
        fz_drop_buffer(ctx, buffer);
        fz_drop_context(ctx);
        return QPixmap();
    }
    fz_output *out = fz_new_output_with_buffer(ctx, buffer);
    fz_try(ctx)
        fz_write_pixmap_as_pnm(ctx, out, pixmap);
    fz_catch(ctx)
    {
        qWarning() << "Failed to write PNM image to buffer:" << fz_caught_message(ctx);
        fz_clear_buffer(ctx, buffer);
        fz_drop_buffer(ctx, buffer);
        fz_drop_output(ctx, out);
        fz_drop_pixmap(ctx, pixmap);
        fz_drop_context(ctx);
        return QPixmap();
    }
    fz_drop_output(ctx, out);
    fz_drop_pixmap(ctx, pixmap);

    // Load the pixmap from buffer in Qt.
    QPixmap qpixmap;
    if (!buffer || !qpixmap.loadFromData(buffer->data, buffer->len, "PNM"))
    {
        qWarning() << "Failed to load PNM image from buffer";
    }
    fz_clear_buffer(ctx, buffer);
    fz_drop_buffer(ctx, buffer);
    fz_drop_context(ctx);
    return qpixmap;
}

const PngPixmap * MuPdfRenderer::renderPng(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return nullptr;

    // Let the main thread prepare everything.
    fz_context *ctx = nullptr;
    fz_rect bbox;
    fz_display_list *list = nullptr;
    doc->prepareRendering(&ctx, &bbox, &list, page, resolution);

    // If page is not valid (too large), the nullptrs will be unchanged.
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

    // Create a new pixmap and fill it with background color (white).
    // TODO: check if this causes problems with different PDF background colors.
    fz_pixmap *pixmap = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_round_rect(bbox), NULL, 0);
    fz_clear_pixmap_with_value(ctx, pixmap, 0xff);

    // Create a device for rendering the given display list to pixmap.
    fz_device *dev = fz_new_draw_device(ctx, fz_identity, pixmap);

    // Do the main work: Render the display list to pixmap.
    fz_try(ctx)
        fz_run_display_list(ctx, list, dev, fz_identity, bbox, NULL);
    fz_catch(ctx)
    {
        qWarning() << "Fitz failed to render pixmap:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_drop_display_list(ctx, list);
        fz_close_device(ctx, dev);
        fz_drop_device(ctx, dev);
        fz_drop_context(ctx);
        return nullptr;
    }

    // Clean up device and list.
    fz_drop_display_list(ctx, list);
    fz_close_device(ctx, dev);
    fz_drop_device(ctx, dev);

    // Save the pixmap to buffer in PNG format.
    fz_buffer *buffer = nullptr;
    fz_try(ctx)
        buffer = fz_new_buffer_from_pixmap_as_png(ctx, pixmap, fz_default_color_params);
    fz_catch(ctx)
    {
        qWarning() << "Fitz failed to write pixmap to PNG buffer:" << fz_caught_message(ctx);
        fz_clear_buffer(ctx, buffer);
        fz_drop_buffer(ctx, buffer);
        fz_drop_context(ctx);
        return nullptr;
    }
    fz_drop_pixmap(ctx, pixmap);

    // Convert the buffer data to QByteArray.
    const QByteArray * data = buffer ? new QByteArray(reinterpret_cast<const char*>(buffer->data), buffer->len) : nullptr;
    fz_clear_buffer(ctx, buffer);
    fz_drop_buffer(ctx, buffer);
    fz_drop_context(ctx);
    return new PngPixmap(data, page, resolution);
}
