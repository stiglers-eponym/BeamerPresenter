#include "src/rendering/mupdfdocument.h"
#include "src/enumerates.h"

std::string roman(int number)
{
    int tens = number / 10;
    std::string roman;
    while (tens-- > 0)
        roman.push_back('x');
    int residual = number % 10;
    if (residual == 9)
        roman.append("ix");
    else if (residual >= 5)
    {
        roman.push_back('v');
        while (residual-- > 5)
            roman.push_back('i');
    }
    else if (residual == 4)
        roman.append("iv");
    else
        while (residual-- > 0)
            roman.push_back('i');
    return roman;
}

std::string decode_pdf_label(int number, const label_item &item)
{
    std::string string = item.prefix ? item.prefix : "";
    if (!item.style)
        return string;
    number += item.start_value;
    switch (*item.style)
    {
    case '\0':
        break;
    case 'r':
        string.append(roman(number));
        break;
    case 'R':
    {
        std::string lower = roman(number);
        for (auto &it : lower)
            it += 0x20;
        string.append(lower);
        break;
    }
    case 'a':
    {
        int repetitions = number / 26 + 1;
        while (repetitions-- > 0)
            string.push_back(0x61 + (number % 26));
        break;
    }
    case 'A':
    {
        int repetitions = number / 26 + 1;
        while (repetitions-- > 0)
            string.push_back(0x41 + (number % 26));
        break;
    }
    default:
        // Decimal is the default.
        string.append(std::to_string(number));
        break;
    }
    //qDebug() << number << QString::fromStdString(string);
    return string;
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

MuPdfDocument::~MuPdfDocument()
{
    mutex->lock();
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    qDeleteAll(mutex_list);
    mutex->unlock();
    delete mutex;
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
    if (doc)
    {
        if (fileinfo.lastModified() == lastModified)
            return false;
        mutex->lock();
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        flexible_page_sizes = -1;
    }
    else
        mutex->lock();

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
    {
        // TODO: find out how to check system preferences for encoding
        //const QByteArray &pathdecoded = path.toLatin1();
        const QByteArray &pathdecoded = path.toUtf8();
        const char *name = pathdecoded.data();
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
    }

    // Save the modification time.
    lastModified = fileinfo.lastModified();

    // Save number of pages.
    number_of_pages = fz_count_pages(ctx, doc);
    mutex->unlock();

    // Load page labels.
    loadPageLabels();

    qDebug() << "Loaded PDF document in MuPDF";
    return number_of_pages > 0;
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

const QString MuPdfDocument::pageLabel(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return "";

    if (pageLabels.isEmpty())
        return QString::number(page);
    return (--pageLabels.upperBound(page)).value();
}

int MuPdfDocument::pageIndex(const QString &page) const
{
    if (pageLabels.isEmpty())
        return page.toInt() - 1;
    // This is slow (linear time):
    return pageLabels.key(page, -1);
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

    pdf_document *pdf_doc = nullptr;
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
        const int len_minus_one = pdf_array_len(ctx, nums) - 1;
        pdf_obj *val;
        // TODO: check if this implementation is reasonable.
        auto pdf_dict_to_int_default = [&](pdf_obj* obj, const char* key, int def)->int{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_int(ctx, value)) ? pdf_to_int(ctx, value) : def;
        };
        auto pdf_dict_to_name = [&](pdf_obj* obj, const char* key)->const char*{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_name(ctx, value)) ? pdf_to_name(ctx, value) : nullptr;
        };
        auto pdf_dict_to_string = [&](pdf_obj* obj, const char* key)->const char*{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_string(ctx, value)) ? pdf_to_text_string(ctx, value) : nullptr;
        };
        for (int i = 0, key; i < len_minus_one;)
        {
            key = pdf_array_get_int(ctx, nums, i++);
            // Actually the following condition should never become true.
            // However, I have found a PDF (generated with LaTeX beamer), for
            // which this is relevant.
            if (key >= number_of_pages || key < 0)
                break;
            val = pdf_array_get(ctx, nums, i++);

            if (pdf_is_dict(ctx, val))
            {
                raw_labels.insert(
                        key,
                        {
                            pdf_dict_to_name(val, "S"),
                            pdf_dict_to_string(val, "P"),
                            pdf_dict_to_int_default(val, "St", 1),
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
    for (auto it = raw_labels.cbegin(); it != raw_labels.cend(); ++it)
    {
        // Check if style is empty, which indicates that all following pages
        // will have the same label.
        if (!it->style || *it->style == '\0')
        {
            // This should be the default case for presentations created with
            // LaTeX beamer.
            pageLabels[it.key()] = it->prefix ? QString::fromLatin1(it->prefix) : "";
        }
        else
        {
            int i = it.key();
            const int next_num = (it+1 == raw_labels.cend()) ? number_of_pages : (it+1).key();
            for (; i < next_num; i++)
                pageLabels[i] = QString::fromStdString(decode_pdf_label(i - it.key(), *it));
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
    if (pagenumber < 0 || pagenumber >= number_of_pages || resolution <= 0.)
        return;

    // This is almost completely copied from a mupdf example.

    mutex->lock();
    // sender gets a references to context.
    *context = ctx;
    // Get a page (must be done in the main thread!).
    // This causes warnings if the page contains multimedia content.
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

const SlideTransition MuPdfDocument::transition(const int page) const
{
    SlideTransition trans;
    if (page < 0 || page >= number_of_pages)
        return trans;

    fz_transition *doc_trans = new fz_transition();
    mutex->lock();
    fz_page *doc_page = fz_load_page(ctx, doc, page);
    fz_page_presentation(ctx, doc_page, doc_trans, nullptr);
    if (doc_trans)
    {
        trans.properties = (doc_trans->vertical ? SlideTransition::Vertical : 0)
                        | (doc_trans->outwards ? SlideTransition::Outwards : 0);
        trans.type = static_cast<SlideTransition::Type>(doc_trans->type);
        trans.angle = doc_trans->direction;
        trans.duration = doc_trans->duration;
        if (trans.type == SlideTransition::Fly)
        {
            pdf_page *pdfpage = pdf_page_from_fz_page(ctx, doc_page);
            pdf_obj *transdict = pdf_dict_get(ctx, pdfpage->obj, PDF_NAME(Trans));
            if (pdf_dict_get_bool(ctx, transdict, PDF_NAME(B)))
                trans.type = SlideTransition::FlyRectangle;
            pdf_obj *ss_obj = pdf_dict_gets(ctx, transdict, "SS");
            if (ss_obj)
                trans.scale = pdf_to_real(ctx, ss_obj);
        }
    }
    fz_drop_page(ctx, doc_page);
    mutex->unlock();
    delete doc_trans; // TODO: Does this break something?
    return trans;
}

const PdfLink MuPdfDocument::linkAt(const int page, const QPointF &position) const
{
    PdfLink result {NoLink, ""};
    if (page < 0 || page >= number_of_pages)
        return result;
    mutex->lock();
    fz_page *doc_page = fz_load_page(ctx, doc, page);
    // TODO: check how this is correctly tidied up!
    fz_link * const clink = fz_load_links(ctx, doc_page);
    for (fz_link* link = clink; link != nullptr; link = link->next)
    {
        if (link->rect.x0 <= position.x() && link->rect.x1 >= position.x() && link->rect.y0 <= position.y() && link->rect.y1 >= position.y())
        {
            if (link->uri == nullptr)
                result = {NoLink, ""};
            else if (link->uri[0] == '#')
            {
                // Internal navigation link
                float x, y;
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

const VideoAnnotation MuPdfDocument::annotationAt(const int page, const QPointF &position) const
{
    if (page < 0 || page >= number_of_pages)
        return VideoAnnotation();
    mutex->lock();
    pdf_page *pdfpage = pdf_load_page(ctx, pdf_document_from_fz_document(ctx, doc), page);
    for (pdf_annot *annot = pdfpage->annots; annot != nullptr; annot = annot->next)
    {
        pdf_keep_annot(ctx, annot); // is this necessary?
        fz_rect bound = pdf_bound_annot(ctx, annot);
        if (bound.x0 <= position.x() && bound.x1 >= position.x() && bound.y0 <= position.y() && bound.y1 >= position.y())
        {
            switch (pdf_annot_type(ctx, annot))
            {
            case PDF_ANNOT_MOVIE:
            {
                qDebug() << "Movie annotation";
                pdf_obj *movie_obj = pdf_dict_gets(ctx, annot->obj, "Movie");
                if (!movie_obj)
                {
                    qWarning() << "Error while reading movie annotation";
                    break;
                }
                const QString file = pdf_dict_get_text_string(ctx, movie_obj, PDF_NAME(F));
                VideoAnnotation videoAnnotation {
                            QUrl::fromLocalFile(file),
                            VideoAnnotation::Once,
                            QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0)
                        };
                pdf_obj *activation_obj = pdf_dict_get(ctx, annot->obj, PDF_NAME(A));
                if (activation_obj)
                {
                    QString mode = pdf_to_name(ctx, pdf_dict_gets(ctx, activation_obj, "Mode") );
                    if (!mode.isEmpty())
                    {
                        if (mode == "Open")
                            videoAnnotation.mode = VideoAnnotation::Open;
                        else if (mode == "Palindrome")
                            videoAnnotation.mode = VideoAnnotation::Palindrome;
                        else if (mode == "Repeat")
                            videoAnnotation.mode = VideoAnnotation::Repeat;
                    }
                }
                qDebug() << videoAnnotation.file << videoAnnotation.mode;
                pdf_drop_annot(ctx, annot);
                mutex->unlock();
                return videoAnnotation;
            }
            case PDF_ANNOT_SOUND:
                qDebug() << "Sound annotation";
                break;
            case PDF_ANNOT_SCREEN:
                qDebug() << "Screen annotation";
                break;
            default:
            qDebug() << "Annotation type:" << pdf_string_from_annot_type(ctx, pdf_annot_type(ctx, annot));
            }
        }
        pdf_drop_annot(ctx, annot);
    }
    fz_drop_page(ctx, (fz_page*) pdfpage);
    mutex->unlock();
    return {QUrl(), VideoAnnotation::Invalid, QRectF()};
}

QList<VideoAnnotation> *MuPdfDocument::annotations(const int page) const
{
    QList<VideoAnnotation>* list = nullptr;
    if (page < 0 || page >= number_of_pages)
        return list;
    mutex->lock();
    pdf_page *pdfpage = pdf_load_page(ctx, pdf_document_from_fz_document(ctx, doc), page);
    for (pdf_annot *annot = pdfpage->annots; annot != nullptr; annot = annot->next)
    {
        if (pdf_annot_type(ctx, annot) == PDF_ANNOT_MOVIE)
        {
            pdf_keep_annot(ctx, annot); // is this necessary?
            pdf_obj *movie_obj = pdf_dict_gets(ctx, annot->obj, "Movie");
            if (!movie_obj)
            {
                qWarning() << "Error while reading movie annotation";
                break;
            }
            const QString file = pdf_dict_get_text_string(ctx, movie_obj, PDF_NAME(F));
            fz_rect bound = pdf_bound_annot(ctx, annot);
            if (list == nullptr)
                list = new QList<VideoAnnotation>();
            list->append({
                        QUrl::fromLocalFile(file),
                        VideoAnnotation::Once,
                        QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0)
                    });
            pdf_obj *activation_obj = pdf_dict_get(ctx, annot->obj, PDF_NAME(A));
            if (activation_obj)
            {
                QString mode = pdf_to_name(ctx, pdf_dict_gets(ctx, activation_obj, "Mode") );
                if (!mode.isEmpty())
                {
                    if (mode == "Open")
                        list->last().mode = VideoAnnotation::Open;
                    else if (mode == "Palindrome")
                        list->last().mode = VideoAnnotation::Palindrome;
                    else if (mode == "Repeat")
                        list->last().mode = VideoAnnotation::Repeat;
                }
            }
            pdf_drop_annot(ctx, annot);
        }
    }
    fz_drop_page(ctx, (fz_page*) pdfpage);
    mutex->unlock();
    return list;
}

bool MuPdfDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || doc == nullptr)
        return flexible_page_sizes;
    flexible_page_sizes = 0;
    mutex->lock();
    // Load page.
    fz_page *doc_page = fz_load_page(ctx, doc, 0);
    // Get bounding box.
    const fz_rect ref_bbox = fz_bound_page(ctx, doc_page);
    fz_rect bbox;
    // Clean up page.
    fz_drop_page(ctx, doc_page);
    for (int page=1; page<number_of_pages; page++)
    {
        // Load page.
        doc_page = fz_load_page(ctx, doc, page);
        // Get bounding box.
        bbox = fz_bound_page(ctx, doc_page);
        // Clean up page.
        fz_drop_page(ctx, doc_page);
        if (bbox.x1 != ref_bbox.x1 || bbox.y1 != ref_bbox.y1)
        {
            flexible_page_sizes = 1;
            break;
        }
    }
    mutex->unlock();
    return flexible_page_sizes;
}

void MuPdfDocument::loadOutline()
{
    // TODO: a huge outline will probably lead to a crash of the program.
    outline.clear();
    outline.append({"", -1, 1});
    mutex->lock();
    fz_outline *root = fz_load_outline(ctx, doc);
    if (root)
    {
        // dangerous anonymous recursion
        auto fill_outline = [&](fz_outline *entry, auto& function) -> void
        {
            const int idx = outline.length();
            outline.append({entry->title, entry->page, -1});
            if (entry->down)
                function(entry->down, function);
            if (entry->next)
            {
                outline[idx].next = outline.length();
                function(entry->next, function);
            }
            else
                outline[idx].next = -outline.length();
        };
        fill_outline(root, fill_outline);
        fz_drop_outline(ctx, root);
    }
    mutex->unlock();
    for (int i=0; i<outline.length(); i++)
        qDebug() << i << outline[i].page << outline[i].next << outline[i].title;
}
