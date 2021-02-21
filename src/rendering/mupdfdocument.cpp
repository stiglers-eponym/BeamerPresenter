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
    debug_verbose(DebugRendering) << number << QString::fromStdString(string);
    return string;
}


void lock_mutex(void *user, int lock)
{
    QVector<QMutex*> *mutex = static_cast<QVector<QMutex*>*>(user);
    (*mutex)[lock]->lock();
}


void unlock_mutex(void *user, int lock)
{
    QVector<QMutex*> *mutex = static_cast<QVector<QMutex*>*>(user);
    (*mutex)[lock]->unlock();
}


MuPdfDocument::MuPdfDocument(const QString &filename) :
    PdfDocument(filename),
    mutex(new QMutex())
{
    for (auto &it : mutex_list)
        it = new QMutex();
    // Load the document
    if (!loadDocument() && !doc)
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
    if (doc && fileinfo.lastModified() == lastModified)
        return false;
    mutex->lock();
    if (doc)
    {
        fz_drop_document(ctx, doc);
        flexible_page_sizes = -1;
    }
    else
    {
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

        if (ctx == NULL)
        {
            qCritical() << "Failed to create Fitz ctx";
            doc = NULL;
            mutex->unlock();
            return false;
        }

        // Try to register default document handlers.
        fz_try(ctx)
            fz_register_document_handlers(ctx);
        fz_catch(ctx)
        {
            qCritical() << "MuPdf cannot register document handlers:" << fz_caught_message(ctx);
            doc = NULL;
            fz_drop_context(ctx);
            ctx =  NULL;
            mutex->unlock();
            return false;
        }
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
            qCritical() << "MuPdf cannot open document:" << fz_caught_message(ctx);
            doc = NULL;
            fz_drop_context(ctx);
            ctx =  NULL;
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

    debug_msg(DebugRendering) << "Loaded PDF document in MuPDF";
    return number_of_pages > 0;
}

const QSizeF MuPdfDocument::pageSize(const int page) const
{
    // Check if the page number is valid.
    if (page < 0 || page >= number_of_pages)
        return QSizeF();

    mutex->lock();
    fz_page *doc_page = NULL;
    fz_rect bbox;
    fz_var(doc_page);
    fz_var(bbox);
    fz_try(ctx)
    {
        // Load page.
        doc_page = fz_load_page(ctx, doc, page);
        // Get bounding box.
        bbox = fz_bound_page(ctx, doc_page);
    }
    fz_always(ctx)
    {
        fz_drop_page(ctx, doc_page);
        mutex->unlock();
    }
    fz_catch(ctx)
        return QSizeF();

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
        return QString::number(page + 1);
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
    if (!ctx || !doc)
        return;
    // (My comments here are based on an old version of MuPDF)
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

    pdf_document *pdf_doc = NULL;
    fz_var(pdf_doc);
    fz_try(ctx)
        pdf_doc = pdf_document_from_fz_document(ctx, doc);
    fz_catch(ctx)
    {
        qWarning() << "Failed to interpret document as PDF" << fz_caught_message(ctx);
        mutex->unlock();
        return;
    }

    QMap<int, label_item> raw_labels;
    fz_try(ctx)
    {
        pdf_obj *labels = pdf_dict_gets(ctx, pdf_dict_gets(ctx, pdf_trailer(ctx, pdf_doc), "Root"), "PageLabels");
        if (!pdf_is_dict(ctx, labels))
            fz_throw(ctx, 1, "document contains no labels");

        pdf_obj *nums = pdf_dict_gets(ctx, labels, "Nums");
        if (!pdf_is_array(ctx, nums))
            fz_throw(ctx, 2, "document contains no valid labels");

        // Read the raw labels from the PDF
        const int len_minus_one = pdf_array_len(ctx, nums) - 1;
        pdf_obj *val;

        // TODO: check if this implementation is reasonable.
        auto pdf_dict_to_int_default = [&](pdf_obj* obj, const char* key, int def)->int{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_int(ctx, value)) ? pdf_to_int(ctx, value) : def;
        };
        auto pdf_dict_to_name = [&](pdf_obj* obj, const char* key)->const char*{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_name(ctx, value)) ? pdf_to_name(ctx, value) : NULL;
        };
        auto pdf_dict_to_string = [&](pdf_obj* obj, const char* key)->const char*{
            pdf_obj *value = pdf_dict_gets(ctx, obj, key);
            return (value && pdf_is_string(ctx, value)) ? pdf_to_text_string(ctx, value) : NULL;
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
    fz_always(ctx)
        mutex->unlock();
    fz_catch(ctx)
    {
        qWarning() << "Failed to load page labels:" << fz_caught_message(ctx);
        return;
    }

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
    if (pagenumber < 0 || pagenumber >= number_of_pages || resolution <= 0. || !ctx || !doc)
        return;

    // This is almost completely copied from a mupdf example.

    mutex->lock();
    // sender gets a references to context.
    *context = ctx;
    // Get a page (must be done in the main thread!).
    // This causes warnings if the page contains multimedia content.
    fz_page *page = NULL;
    fz_var(page);
    fz_var(*bbox);
    fz_try(ctx)
    {
        page = fz_load_page(ctx, doc, pagenumber);
        *bbox = fz_bound_page(ctx, page);
    }
    fz_catch(ctx)
    {
        fz_drop_page(ctx, page);
        mutex->unlock();
        return;
    }
    // Calculate the boundary box and rescale it to the given resolution.
    // bbox is now given in points. Convert to pixels using resolution, which
    // is given in pixels per point.
    bbox->x0 *= resolution;
    bbox->x1 *= resolution;
    bbox->y0 *= resolution;
    bbox->y1 *= resolution;

    fz_device *dev = NULL;
    fz_var(*list);
    fz_var(dev);
    fz_try(ctx)
    {
        // Prepare a display list for a drawing device.
        // The list (and not the page itself) will then be used to render the page.
        *list = fz_new_display_list(ctx, *bbox);
        // Use a fitz device to fill the list with the content of the page.
        dev = fz_new_list_device(ctx, *list);
        fz_run_page_contents(ctx, page, dev, fz_scale(resolution, resolution), NULL);
    }
    fz_always(ctx)
    {
        fz_drop_device(ctx, dev);
        fz_drop_page(ctx, page);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        *list = NULL;
    }
}

const SlideTransition MuPdfDocument::transition(const int page) const
{
    SlideTransition trans;
    if (page < 0 || page >= number_of_pages || !ctx || !doc)
        return trans;

    mutex->lock();
    fz_page *doc_page = NULL;
    fz_var(doc_page);
    fz_try(ctx)
        doc_page = fz_load_page(ctx, doc, page);
    fz_catch(ctx)
    {
        fz_drop_page(ctx, doc_page);
        mutex->unlock();
        return trans;
    }

    fz_transition doc_trans;
    fz_var(doc_trans);
    fz_try(ctx)
        fz_page_presentation(ctx, doc_page, &doc_trans, NULL);
    fz_catch(ctx)
    {
        fz_drop_page(ctx, doc_page);
        mutex->unlock();
        return trans;
    }

    trans.properties = (doc_trans.vertical ? SlideTransition::Vertical : 0)
                    | (doc_trans.outwards ? SlideTransition::Outwards : 0);
    trans.type = static_cast<SlideTransition::Type>(doc_trans.type);
    trans.angle = doc_trans.direction;
    trans.duration = doc_trans.duration;

    if (trans.type == SlideTransition::Fly)
    {
        fz_var(trans.type);
        fz_var(trans.scale);
        fz_try(ctx)
        {
            pdf_page *pdfpage = pdf_page_from_fz_page(ctx, doc_page);
            pdf_obj *transdict = pdf_dict_get(ctx, pdfpage->obj, PDF_NAME(Trans));
            if (pdf_dict_get_bool(ctx, transdict, PDF_NAME(B)))
                trans.type = SlideTransition::FlyRectangle;
            pdf_obj *ss_obj = pdf_dict_gets(ctx, transdict, "SS");
            if (ss_obj)
                trans.scale = pdf_to_real(ctx, ss_obj);
        }
        fz_catch(ctx)
             warn_msg << "failed to completely load slide transition" << fz_caught_message(ctx);
    }

    fz_drop_page(ctx, doc_page);
    mutex->unlock();
    return trans;
}

const PdfLink MuPdfDocument::linkAt(const int page, const QPointF &position) const
{
    PdfLink result {NoLink, "", QRectF()};
    if (page < 0 || page >= number_of_pages || !ctx || !doc)
        return result;

    mutex->lock();
    fz_page *doc_page = NULL;
    fz_var(doc_page);
    fz_try(ctx)
        doc_page = fz_load_page(ctx, doc, page);
    fz_catch(ctx)
    {
        fz_drop_page(ctx, doc_page);
        mutex->unlock();
        return result;
    }

    // TODO: check how this is correctly tidied up!
    fz_link *clink = NULL;
    fz_var(clink);
    fz_try(ctx)
    {
        clink = fz_load_links(ctx, doc_page);
        for (fz_link* link = clink; link != NULL; link = link->next)
        {
            if (link->rect.x0 <= position.x() && link->rect.x1 >= position.x() && link->rect.y0 <= position.y() && link->rect.y1 >= position.y())
            {
                const QRectF rect = QRectF(link->rect.x0, link->rect.y0, link->rect.x1 - link->rect.x0, link->rect.y1 - link->rect.y0).normalized();
                if (link->uri == NULL)
                    result = {NoLink, "", rect};
                else if (link->uri[0] == '#')
                {
                    // Internal navigation link
                    float x, y;
#if (FZ_VERSION_MAJOR >= 1)  && (FZ_VERSION_MINOR >= 17)
                    fz_location location = fz_resolve_link(ctx, doc, link->uri, &x, &y);
                    result = {location.page, "", rect};
#else
                    const int page = fz_resolve_link(ctx, doc, link->uri, &x, &y);
                    result = {page, ""};
#endif
                }
                else
                {
                    debug_msg(DebugRendering) << "Unsupported link" << link->uri;
                    result = {NoLink, "", rect};
                }
                break;
            }
        }
    }
    fz_always(ctx)
    {
        fz_drop_link(ctx, clink);
        fz_drop_page(ctx, doc_page);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        warn_msg << "Error while loading link" << fz_caught_message(ctx);
    }
    return result;
}

const MediaAnnotation MuPdfDocument::annotationAt(const int page, const QPointF &position) const
{
    MediaAnnotation result = {QUrl(), MediaAnnotation::InvalidAnnotation, MediaAnnotation::Invalid, QRectF()};
    if (page < 0 || page >= number_of_pages || !ctx || !doc)
        return result;

    mutex->lock();
    pdf_page *pdfpage = NULL;
    fz_var(pdfpage);
    fz_try(ctx)
    {
        pdfpage = pdf_load_page(ctx, pdf_document_from_fz_document(ctx, doc), page);
        for (pdf_annot *annot = pdfpage->annots; annot != NULL; annot = annot->next)
        {
            fz_rect bound = pdf_bound_annot(ctx, annot);
            if (bound.x0 <= position.x() && bound.x1 >= position.x() && bound.y0 <= position.y() && bound.y1 >= position.y())
            {
                switch (pdf_annot_type(ctx, annot))
                {
                case PDF_ANNOT_MOVIE:
                case PDF_ANNOT_SOUND:
                {
                    debug_msg(DebugMedia) << "Movie annotation";
                    pdf_obj *media_obj = pdf_dict_gets(ctx, annot->obj, "Movie");
                    if (!media_obj)
                    {
                        qWarning() << "Error while reading media annotation";
                        break;
                    }
                    result.type = pdf_annot_type(ctx, annot) == PDF_ANNOT_MOVIE ? MediaAnnotation::VideoAnnotation : MediaAnnotation::AudioAnnotation;
                    const QFileInfo fileinfo(pdf_dict_get_text_string(ctx, media_obj, PDF_NAME(F)));
                    //pdf_drop_obj(ctx, media_obj);
                    if (!fileinfo.exists())
                        continue;
                    result.file = QUrl::fromLocalFile(fileinfo.absoluteFilePath());
                    result.rect = QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0);

                    pdf_obj *activation_obj = pdf_dict_get(ctx, annot->obj, PDF_NAME(A));
                    if (activation_obj)
                    {
                        QString mode = pdf_to_name(ctx, pdf_dict_gets(ctx, activation_obj, "Mode") );
                        if (!mode.isEmpty())
                        {
                            if (mode == "Open")
                                result.mode = MediaAnnotation::Open;
                            else if (mode == "Palindrome")
                                result.mode = MediaAnnotation::Palindrome;
                            else if (mode == "Repeat")
                                result.mode = MediaAnnotation::Repeat;
                        }
                        //pdf_drop_obj(ctx, activation_obj);
                    }
                    break;
                }
                case PDF_ANNOT_SCREEN:
                    debug_msg(DebugMedia) << "Screen annotation";
                    break;
                default:
                debug_msg(DebugRendering) << "Annotation type:" << pdf_string_from_annot_type(ctx, pdf_annot_type(ctx, annot));
                }
            }
            if (result.type != MediaAnnotation::InvalidAnnotation)
                break;
        }
    }
    fz_always(ctx)
    {
        fz_drop_page(ctx, (fz_page*) pdfpage);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        return result;
    }
    debug_msg(DebugMedia) << result.file << result.mode;
    return result;
}

QList<MediaAnnotation> *MuPdfDocument::annotations(const int page) const
{
    QList<MediaAnnotation>* list = NULL;
    if (page < 0 || page >= number_of_pages || !ctx || !doc)
        return list;
    mutex->lock();
    pdf_page *pdfpage = NULL;
    fz_var(pdfpage);
    fz_try(ctx)
        pdfpage = pdf_load_page(ctx, pdf_document_from_fz_document(ctx, doc), page);
    fz_catch(ctx)
    {
        fz_drop_page(ctx, (fz_page*) pdfpage);
        mutex->unlock();
        return list;
    }
    fz_var(list);
    fz_try(ctx)
    {
        for (pdf_annot *annot = pdfpage->annots; annot != NULL; annot = annot->next)
        {
            switch (pdf_annot_type(ctx, annot))
            {
            case PDF_ANNOT_MOVIE:
            case PDF_ANNOT_SOUND:
            {
                pdf_obj *media_obj = pdf_dict_gets(ctx, annot->obj, "Movie");
                if (!media_obj)
                {
                    qWarning() << "Error while reading media annotation";
                    break;
                }
                const QFileInfo fileinfo(pdf_dict_get_text_string(ctx, media_obj, PDF_NAME(F)));
                //pdf_drop_obj(ctx, media_obj);
                if (!fileinfo.exists())
                    continue;
                fz_rect bound = pdf_bound_annot(ctx, annot);
                if (list == NULL)
                    list = new QList<MediaAnnotation>();
                list->append({
                            QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                            pdf_annot_type(ctx, annot) == PDF_ANNOT_MOVIE ? MediaAnnotation::VideoAnnotation : MediaAnnotation::AudioAnnotation,
                            MediaAnnotation::Once,
                            QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0)
                        });
                pdf_obj *activation_obj = pdf_dict_get(ctx, annot->obj, PDF_NAME(A));
                if (activation_obj)
                {
                    QString mode = pdf_to_name(ctx, pdf_dict_gets(ctx, activation_obj, "Mode") );
                    if (!mode.isEmpty())
                    {
                        if (mode == "Open")
                            list->last().mode = MediaAnnotation::Open;
                        else if (mode == "Palindrome")
                            list->last().mode = MediaAnnotation::Palindrome;
                        else if (mode == "Repeat")
                            list->last().mode = MediaAnnotation::Repeat;
                    }
                    //pdf_drop_obj(ctx, activation_obj);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    fz_always(ctx)
    {
        fz_drop_page(ctx, (fz_page*) pdfpage);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        warn_msg << "Error in searching annotations:" << fz_caught_message(ctx);
    }
    return list;
}

bool MuPdfDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || !ctx || !doc)
        return flexible_page_sizes;

    flexible_page_sizes = 0;
    mutex->lock();
    fz_page *doc_page;
    fz_var(doc_page);
    fz_try(ctx)
    {
        // Load page.
        doc_page = fz_load_page(ctx, doc, 0);
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
        doc_page = NULL;
    }
    fz_always(ctx)
    {
        if (doc_page)
            fz_drop_page(ctx, doc_page);
        mutex->unlock();
    }
    fz_catch(ctx)
        qWarning() << "Error while checking page sizes" << fz_caught_message(ctx);
    return flexible_page_sizes;
}

void MuPdfDocument::loadOutline()
{
    if (!ctx || !doc)
        return;

    // TODO: a huge outline will probably lead to a crash of the program.
    outline.clear();
    outline.append({"", -1, 1});
    mutex->lock();

    fz_outline *root;
    fz_var(root);
    fz_try(ctx)
    {
        root = fz_load_outline(ctx, doc);
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
        }
    }
    fz_always(ctx)
    {
        fz_drop_outline(ctx, root);
        mutex->unlock();
    }
    fz_catch(ctx)
        qWarning() << "Error while loading outline" << fz_caught_message(ctx);

#ifdef QT_DEBUG
    if ((preferences()->log_level & (DebugRendering|DebugVerbose)) == (DebugRendering|DebugVerbose))
        for (int i=0; i<outline.length(); i++)
            qDebug() << DebugRendering << i << outline[i].page << outline[i].next << outline[i].title;
#endif
}

qreal MuPdfDocument::duration(const int page) const noexcept
{
    if (page < 0 || page >= number_of_pages || !ctx || !doc)
        return -1.;
    mutex->lock();
    pdf_page *pdfpage = NULL;
    fz_var(pdfpage);
    fz_try(ctx)
        pdfpage = pdf_load_page(ctx, pdf_document_from_fz_document(ctx, doc), page);
    fz_catch(ctx)
    {
        fz_drop_page(ctx, (fz_page*) pdfpage);
        mutex->unlock();
        return -1.;
    }
    qreal duration;
    fz_var(duration);
    fz_try(ctx)
    {
        pdf_obj *obj = pdf_dict_get(ctx, pdfpage->obj, PDF_NAME(Dur));
        duration = obj ? pdf_to_real(ctx, obj) : -1.;
    }
    fz_always(ctx)
    {
        fz_drop_page(ctx, (fz_page*) pdfpage);
        mutex->unlock();
    }
    fz_catch(ctx)
        duration = -1.;
    return duration;
}
