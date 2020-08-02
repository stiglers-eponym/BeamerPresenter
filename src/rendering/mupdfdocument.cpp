#include "src/rendering/mupdfdocument.h"
#include "src/enumerates.h"

MuPdfDocument::MuPdfDocument(const QString &filename) :
    PdfDocument(filename),
    mutex(new QMutex())
{
    for (auto &it : mutex_list)
        it = new QMutex();
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");
}


void lock_mutex(void *user, int lock)
{
    QVector<QMutex*> *mutex = static_cast<QVector<QMutex*>*>(user);
    //qDebug() << "lock  " << user << lock;
    (*mutex)[lock]->lock();
}


void unlock_mutex(void *user, int lock)
{
    QVector<QMutex*> *mutex = static_cast<QVector<QMutex*>*>(user);
    //qDebug() << "unlock" << user << lock;
    (*mutex)[lock]->unlock();
}


bool MuPdfDocument::loadDocument()
{
    // Check if the file exists.
    QFileInfo const fileinfo(path);
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        qCritical() << "Given filename is not a file.";
        return false;
    }

    mutex->lock();
    // Check if the file has changed since last (re)load
    if (doc != nullptr)
    {
        if (fileinfo.lastModified() == lastModified)
            return false;
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
    }

    // This code is mainly copied from MuPDF example files, see mupdf.com

    // Initialize the locking structure with function pointers to
    // the locking functions and to the user data. In this case
    // the user data is a pointer to the array of mutexes so the
    // locking functions can find the relevant lock to change when
    // they are called. This way we avoid global variables.

    fz_locks_context locks;
    locks.user = &mutex_list;
    locks.lock = lock_mutex;
    locks.unlock = unlock_mutex;

    // This is the main threads ctx function, so supply the
    // locking structure. This ctx will be used to parse all
    // the pages from the document.

    ctx = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);

    if (ctx == nullptr)
    {
        qWarning() << "Failed to create Fitz ctx";
        doc = nullptr;
        mutex->unlock();
        return false;
    }

    // Try to register default document handlers.
    fz_try(ctx)
        fz_register_document_handlers(ctx);
    fz_catch(ctx)
    {
        qWarning() << "MuPdf cannot register document handlers:" << fz_caught_message(ctx);
        doc = nullptr;
        fz_drop_context(ctx);
        ctx =  nullptr;
        mutex->unlock();
        return false;
    }

    // Open the document.
    // TODO: Is it assume that this pointer stays valid?
    // Check again with valgrind / gdb
    const char *name = path.toLatin1().data();
    fz_try(ctx)
        doc = fz_open_document(ctx, name);
    fz_catch(ctx)
    {
        qWarning() << "MuPdf cannot open document:" << fz_caught_message(ctx);
        doc = nullptr;
        fz_drop_context(ctx);
        ctx =  nullptr;
        mutex->unlock();
        return false;
    }

    // Save the modification time.
    lastModified = fileinfo.lastModified();

    // Save number of pages.
    number_of_pages = fz_count_pages(ctx, doc);
    mutex->unlock();

    // Load page labels.
    loadPageLabels();
    return number_of_pages > 0;
}

MuPdfDocument::~MuPdfDocument()
{
    mutex->lock();
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    qDeleteAll(mutex_list);
    mutex->unlock();
    delete mutex;
}

bool MuPdfDocument::isValid() const
{
    return doc != nullptr && ctx != nullptr && number_of_pages > 0;
}

const QPixmap MuPdfDocument::getPixmap(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0 || page >= number_of_pages)
        return QPixmap();

    // TODO: This ignores PagePart.
    qDebug() << "Rendering directly in MuPdfDocument";

    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    mutex->lock();
    fz_try(ctx)
        pixmap = fz_new_pixmap_from_page_number(ctx, doc, page, fz_scale(resolution, resolution), fz_device_rgb(ctx), 0);
    fz_catch(ctx)
    {
        qWarning() << "MuPDF cannot render page:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        mutex->unlock();
        return QPixmap();
    }

    // Assume that the pixmap is in RGB colorspace.
    // Write the pixmap in PNM format to a buffer using MuPDF tools.
    fz_buffer *buffer;
    fz_try(ctx)
        buffer = fz_new_buffer(ctx, pixmap->stride * pixmap->y + 16);
    fz_catch(ctx)
    {
        qWarning() << "Failed to allocate memory in Fitz buffer:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        fz_clear_buffer(ctx, buffer);
        fz_drop_buffer(ctx, buffer);
        mutex->unlock();
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
        mutex->unlock();
        return QPixmap();
    }
    fz_drop_pixmap(ctx, pixmap);

    // Load the pixmap from buffer in Qt.
    QPixmap qpixmap;
    if (!qpixmap.loadFromData(buffer->data, buffer->len, "PNM"))
    {
        qWarning() << "Failed to load PNM image from buffer";
    }
    fz_clear_buffer(ctx, buffer);
    fz_drop_buffer(ctx, buffer);
    mutex->unlock();
    return qpixmap;
}

const PngPixmap * MuPdfDocument::getPng(const int page, const qreal resolution) const
{
    // Check if the parameters are valid.
    if (resolution <= 0 || page < 0 || page >= number_of_pages)
        return nullptr;

    // TODO: This ignores PagePart.
    qDebug() << "Rendering directly in MuPdfDocument";

    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    mutex->lock();
    fz_try(ctx)
        pixmap = fz_new_pixmap_from_page_number(ctx, doc, page, fz_scale(resolution, resolution), fz_device_rgb(ctx), 0);
    fz_catch(ctx)
    {
        qWarning() << "MuPDF cannot render page:" << fz_caught_message(ctx);
        fz_drop_pixmap(ctx, pixmap);
        mutex->unlock();
        return nullptr;
    }

    // Save the pixmap to buffer in PNG format.
    fz_buffer *buffer;
    fz_try(ctx)
        buffer = fz_new_buffer_from_pixmap_as_png(ctx, pixmap, fz_default_color_params);
    fz_catch(ctx)
    {
        qWarning() << "Fitz failed to write pixmap to PNG buffer:" << fz_caught_message(ctx);
        fz_clear_buffer(ctx, buffer);
        fz_drop_buffer(ctx, buffer);
        mutex->unlock();
        return nullptr;
    }

    // Convert the buffer data to QByteArray.
    const QByteArray * data = new QByteArray(reinterpret_cast<const char*>(buffer->data), buffer->len);
    fz_clear_buffer(ctx, buffer);
    mutex->unlock();
    return new PngPixmap(data, page, resolution);
}

const QSizeF MuPdfDocument::pageSize(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return QSizeF();

    mutex->lock();
    // Load page.
    fz_page *doc_page = fz_load_page(ctx, doc, page);
    // Get bounding box.
    const fz_rect bbox = fz_bound_page(ctx, doc_page);
    // Clean up page.
    fz_drop_page(ctx, doc_page);
    mutex->unlock();

    // Convert bounding box to QSizeF.
    // bbox.x0 and bbox.y0 should be 0, but keep them anyway:
    return QSizeF(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);
}

const QString MuPdfDocument::label(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return "";

    if (pageLabels.isEmpty())
        return QString::number(page);
    return (--pageLabels.upperBound(page)).value();
}

int MuPdfDocument::overlaysShifted(const int start, const int shift_overlay) const
{
    // Get the "number" part of shift_overlay by removing the "overlay" flags.
    int shift = shift_overlay >= 0 ? shift_overlay & ~ShiftOverlays::AnyOverlay : shift_overlay | ShiftOverlays::AnyOverlay;
    // Check whether the document has non-trivial page labels and shift has
    // non-trivial overlay flags.
    if (pageLabels.isEmpty() || shift == shift_overlay)
        return start + shift;
    // Find the beginning of next slide.
    QMap<int, QString>::const_iterator it = pageLabels.upperBound(start);
    // Shift the iterator according to shift.
    while (shift > 0 && it != pageLabels.cend())
    {
        --shift;
        ++it;
    }
    while (shift < 0 && it != pageLabels.cbegin())
    {
        ++shift;
        --it;
    }
    // Check if the iterator has reached the beginning or end of the set.
    if (it == pageLabels.cbegin())
        return 0;
    if (it == pageLabels.cend())
    {
        if (shift_overlay & FirstOverlay)
            return (--it).key();
        return number_of_pages - 1;
    }
    // Return first or last overlay depending on overlay flags.
    if (shift_overlay & FirstOverlay)
        return (--it).key();
    return it.key() - 1;
}

void MuPdfDocument::loadPageLabels()
{
    // MuPDF doesn't support page labels (!!!)
    // Here is an implementation which lets MuPdfDocument handle page labels.

    // This function is designed for documents, in which page labels consist
    // only of prefixes. This is the case for presentations created with
    // LaTeX beamer with overlays.
    // Numbering styles other than decimal which are not explicitly given as
    // strings are currently not supported.

    // If you want support for other styles, please open an issue on github.

    mutex->lock();
    // This code is based on a patch by Stefan Radermacher and Albert Bloomfield
    // (which MuPDF should finally include!)
    // https://bugs.ghostscript.com/show_bug.cgi?id=695351

    pdf_document *pdf_doc;
    fz_try(ctx)
        pdf_doc = pdf_document_from_fz_document(ctx, doc);
    fz_catch(ctx)
    {
        qWarning() << "Failed to interpret document as PDF" << fz_caught_message(ctx);
        mutex->unlock();
        return;
    }

    pdf_obj *root = pdf_dict_gets(ctx, pdf_trailer(ctx, pdf_doc), "Root");
    pdf_obj *labels = pdf_dict_gets(ctx, root, "PageLabels");

    if (!pdf_is_dict(ctx, labels))
    {
        qWarning() << "Failed to load page labels.";
        mutex->unlock();
        return;
    }

    pdf_obj *nums = pdf_dict_gets(ctx, labels, "Nums");
    if (!pdf_is_array(ctx, nums))
    {
        qWarning() << "Failed to interpret page labels.";
        mutex->unlock();
        return;
    }

    // Read the raw labels from the PDF
    QMap<int, label_item> raw_labels;
    {
        const int len = pdf_array_len(ctx, nums);

        for (int i = 0; i + 1 < len; i += 2)
        {
            pdf_obj *key = pdf_array_get(ctx, nums, i);
            pdf_obj *val = pdf_array_get(ctx, nums, i + 1);

            if (pdf_is_dict(ctx, val))
            {
                raw_labels.insert(
                            pdf_to_int(ctx, key),
                            {
                                pdf_to_text_string(ctx, pdf_dict_gets(ctx, val, "S")),
                                pdf_to_text_string(ctx, pdf_dict_gets(ctx, val, "P")),
                                pdf_to_int(ctx, pdf_dict_gets(ctx, val, "St"))
                            }
                            );
            }
        }
    }
    mutex->unlock();

    // Check if anything was found.
    if (raw_labels.isEmpty())
    {
        pageLabels.clear();
        return;
    }

    // Currently only decimal style is supported and the style option is
    // ignored.
    for (auto it = raw_labels.cbegin(); it != raw_labels.cend();)
    {
        const QString prefix = QString::fromLatin1(it->prefix);
        if (std::string(it->style).empty())
        {
            // This should be the default case for presentations created with
            // LaTeX beamer.
            pageLabels[it++.key()] = prefix;
        }
        else
        {
            int i = it.key();
            const int next_num = (++it == raw_labels.cend()) ? number_of_pages : it.key();
            for (; i < next_num; i++)
                pageLabels[i] = prefix + QString::number(i);
        }
    }

    // The first label must always be contained in the pageLabels.
    // This should not be necessary, but without that a broken PDF
    // could easily produce segfaults.
    if (pageLabels.firstKey() != 0)
        pageLabels[0] = "";
}

void MuPdfDocument::prepareRendering(fz_context **context, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution) const
{
    // Check if the page number is valid.
    // If it is not, return without changing the given pointers.
    // The caller should note that the pointers are unchaged and handle this
    // appropriately.
    if (pagenumber < 0 || pagenumber >= number_of_pages)
        return;

    // This is almost completely copied from a mupdf example.

    mutex->lock();
    // sender gets a references to context.
    *context = ctx;
    // Get a page (must be done in the main thread!).
    fz_page *page = fz_load_page(ctx, doc, pagenumber);
    // Calculate the boundary box and rescale it to the given resolution.
    *bbox = fz_bound_page(ctx, page);
    // bbox is now given in points. Convert to pixels using resolution, which
    // is given in pixels per point.
    bbox->x0 *= resolution;
    bbox->x1 *= resolution;
    bbox->y0 *= resolution;
    bbox->y1 *= resolution;

    // Prepare a display list for a drawing device.
    // The list (and not the page itself) will then be used to render the page.
    *list = fz_new_display_list(ctx, *bbox);
    // Use a fitz device to fill the list with the content of the page.
    fz_device *dev = fz_new_list_device(ctx, *list);
    fz_run_page(ctx, page, dev, fz_scale(resolution, resolution), NULL);

    // clean up.
    fz_close_device(ctx, dev);
    fz_drop_device(ctx, dev);
    fz_drop_page(ctx, page);
    mutex->unlock();
}

const PdfLink MuPdfDocument::linkAt(const int page, const QPointF &position) const
{
    mutex->lock();
    fz_page *doc_page = fz_load_page(ctx, doc, page);
    // TODO: check how this is correctly tidied up!
    fz_link * const clink = fz_load_links(ctx, doc_page);
    PdfLink result  {NoLink, ""};
    for (fz_link* link = clink; link != nullptr; link = link->next)
    {
        if (link->rect.x0 <= position.x() && link->rect.x1 >= position.x() && link->rect.y0 <= position.y() && link->rect.y1 >= position.y())
        {
            float x, y;
            if (link->uri == nullptr)
                result = {NoLink, ""};
            else if (link->uri[0] == '#')
            {
                // Internal navigation link
                fz_location location = fz_resolve_link(ctx, doc, link->uri, &x, &y);
                result = {location.page, ""};
            }
            else
            {
                qDebug() << "Unsupported link" << link->uri;
                result = {NoLink, ""};
            }
            break;
        }
    }
    fz_drop_link(ctx, clink);
    mutex->unlock();
    return result;
}
