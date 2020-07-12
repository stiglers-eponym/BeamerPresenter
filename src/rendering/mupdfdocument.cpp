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

    const fz_matrix matrix = fz_scale(resolution, resolution);
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

const PngPixmap * MuPdfDocument::getPng(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0 || page >= number_of_pages)
        return nullptr;
    const fz_matrix matrix = fz_scale(resolution, resolution);
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
    return new PngPixmap(data, page, resolution);
}

const QSizeF MuPdfDocument::pageSize(const int page) const
{
    // TODO: check which units this uses
    fz_page *doc_page = fz_load_page(context, doc, page);
    const fz_rect bbox = fz_bound_page(context, doc_page);
    return QSizeF(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);
}

const QString MuPdfDocument::label(const int page) const
{
    // TODO
    return QString::number(page);
}

void  MuPdfDocument::prepareRendering(fz_context **ctx, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution)
{
    *ctx = context;
    fz_page *page = fz_load_page(context, doc, pagenumber);
    *bbox = fz_bound_page(context, page);
    bbox->x0 *= resolution;
    bbox->x1 *= resolution;
    bbox->y0 *= resolution;
    bbox->y1 *= resolution;
    *list = fz_new_display_list(context, *bbox);
    fz_device *dev = fz_new_list_device(context, *list);
    fz_run_page(context, page, dev, fz_scale(resolution, resolution), NULL);
    fz_close_device(context, dev);
    fz_drop_device(context, dev);
    fz_drop_page(context, page);
}
