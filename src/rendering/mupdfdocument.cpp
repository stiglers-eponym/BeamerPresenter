#include "src/rendering/mupdfdocument.h"

MuPdfDocument::MuPdfDocument(const QString &filename) :
    PdfDocument(filename)
{
    for (auto &it : mutex)
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
    // Check if the file has changed since last (re)load
    if (doc != nullptr)
    {
        if (fileinfo.lastModified() == lastModified)
            return false;
        fz_drop_document(context, doc);
        fz_drop_context(context);
    }

    // This code is mainly copied from MuPDF example files, see mupdf.com

    // Initialize the locking structure with function pointers to
    // the locking functions and to the user data. In this case
    // the user data is a pointer to the array of mutexes so the
    // locking functions can find the relevant lock to change when
    // they are called. This way we avoid global variables.

    fz_locks_context locks;
    locks.user = &mutex;
    locks.lock = lock_mutex;
    locks.unlock = unlock_mutex;

    // This is the main threads context function, so supply the
    // locking structure. This context will be used to parse all
    // the pages from the document.

    context = fz_new_context(NULL, &locks, FZ_STORE_UNLIMITED);

    if (context == nullptr)
    {
        qWarning() << "Failed to create Fitz context";
        doc = nullptr;
        return false;
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
        return false;
    }

    // Open the document.
    {
        const char * const name = path.toLatin1().data();
        fz_try(context)
            doc = fz_open_document(context, name);
        fz_catch(context)
        {
            qWarning() << "MuPdf cannot open document:" << fz_caught_message(context);
            doc = nullptr;
            fz_drop_context(context);
            context =  nullptr;
            return false;
        }
    }

    // Save the modification time.
    lastModified = fileinfo.lastModified();

    // Save number of pages.
    number_of_pages = fz_count_pages(context, doc);
    return number_of_pages > 0;
}

MuPdfDocument::~MuPdfDocument()
{
    fz_drop_document(context, doc);
    fz_drop_context(context);
}

bool MuPdfDocument::isValid() const
{
    return doc != nullptr && context != nullptr && number_of_pages > 0;
}

const QPixmap MuPdfDocument::getPixmap(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0 || page >= number_of_pages)
        return QPixmap();

    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc, page, fz_scale(resolution, resolution), fz_device_rgb(context), 0);
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

const PngPixmap * MuPdfDocument::getPng(const int page, const qreal resolution) const
{
    // Check if the parameters are valid.
    if (resolution <= 0 || page < 0 || page >= number_of_pages)
        return nullptr;

    // Render page to an RGB pixmap.
    fz_pixmap *pixmap;
    fz_try(context)
        pixmap = fz_new_pixmap_from_page_number(context, doc, page, fz_scale(resolution, resolution), fz_device_rgb(context), 0);
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

const QSizeF MuPdfDocument::pageSize(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return QSizeF();

    // Load page.
    fz_page *doc_page = fz_load_page(context, doc, page);
    // Get bounding box.
    const fz_rect bbox = fz_bound_page(context, doc_page);
    // Clean up page.
    fz_drop_page(context, doc_page);
    // Convert bounding box to QSizeF.
    // bbox.x0 and bbox.y0 should be 0, but keep them anyway:
    return QSizeF(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);
}

void MuPdfDocument::getPageSize(QSizeF *size, const int page)
{
    *size = pageSize(page);
}

const QString MuPdfDocument::label(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return "";

    // TODO: Find out how to access page labels in MuPDF
    //qWarning() << "Accessing page label is not implemented yet!";
    return QString::number(page);
}

void MuPdfDocument::prepareRendering(fz_context **ctx, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution)
{
    // Check if the page number is valid.
    // If it is not, return without changing the given pointers.
    // The caller should note that the pointers are unchaged and handle this
    // appropriately.
    if (pagenumber < 0 || pagenumber >= number_of_pages)
        return;

    // This is almost completely copied from a mupdf example.

    // sender gets a references to context.
    *ctx = context;
    // Get a page (must be done in the main thread!).
    fz_page *page = fz_load_page(context, doc, pagenumber);
    // Calculate the boundary box and rescale it to the given resolution.
    *bbox = fz_bound_page(context, page);
    // bbox is now given in points. Convert to pixels using resolution, which
    // is given in pixels per point.
    bbox->x0 *= resolution;
    bbox->x1 *= resolution;
    bbox->y0 *= resolution;
    bbox->y1 *= resolution;

    // Prepare a display list for a drawing device.
    // The list (and not the page itself) will then be used to render the page.
    *list = fz_new_display_list(context, *bbox);
    // Use a fitz device to fill the list with the content of the page.
    fz_device *dev = fz_new_list_device(context, *list);
    fz_run_page(context, page, dev, fz_scale(resolution, resolution), NULL);

    // clean up.
    fz_close_device(context, dev);
    fz_drop_device(context, dev);
    fz_drop_page(context, page);
}
