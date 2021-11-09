#include <QFileInfo>
#include <QInputDialog>
#include <QMutex>
#include "src/rendering/mupdfdocument.h"
#include "src/enumerates.h"
#include "src/preferences.h"

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

std::string decode_pdf_label(int number, const MuPdfDocument::label_item &item)
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
    debug_verbose(DebugRendering, number << QString::fromStdString(string));
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
    for (auto page : qAsConst(pages))
        fz_drop_page(ctx, (fz_page*)page);
    pdf_drop_document(ctx, doc);
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
        pdf_drop_document(ctx, doc);
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
            qCritical() << "Failed to create Fitz context";
            doc = NULL;
            mutex->unlock();
            return false;
        }

        // Try to register default document handlers.
        fz_try(ctx)
            fz_register_document_handlers(ctx);
        fz_catch(ctx)
        {
            qCritical() << "MuPdf failed to register document handlers:" << fz_caught_message(ctx);
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
        fz_var(doc);
        fz_try(ctx)
            doc = pdf_open_document(ctx, name);
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
    // Try to decrypt document if necessary
    if (pdf_needs_password(ctx, doc))
    {
        qWarning() << "Document is locked.";
        // Use a QInputDialog to ask for the password.
        bool ok;
        QString const password = QInputDialog::getText(
                    NULL,
                    "Document is locked!",
                    "Please enter password (leave empty to cancel).",
                    QLineEdit::Password,
                    QString(),
                    &ok
                    );
        // Check if a password was entered.
        if (!ok || password.isEmpty() || !pdf_authenticate_password(ctx, doc, password.toUtf8()))
        {
            qCritical() << "No or invalid password provided for locked document";
            pdf_drop_document(ctx, doc);
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
    number_of_pages = pdf_count_pages(ctx, doc);

    qDeleteAll(pages);
    pages.resize(number_of_pages);
    int i=0;
    fz_var(i);
    do {
        fz_var(pages[i]);
        fz_try(ctx)
            pages[i] = pdf_load_page(ctx, doc, i);
        fz_catch(ctx)
            pages[i] = NULL;
    } while (++i < number_of_pages);

    mutex->unlock();

    // Load page labels.
    loadPageLabels();

    debug_msg(DebugRendering, "Loaded PDF document in MuPDF");
    return number_of_pages > 0;
}

const QSizeF MuPdfDocument::pageSize(const int page) const
{
    // Check if the page number is valid.
    if (!pages.value(page))
        return QSizeF();

    mutex->lock();
    fz_rect bbox;
    fz_var(bbox);
    fz_try(ctx)
        // Get bounding box.
        bbox = pdf_bound_page(ctx, pages[page]);
    fz_always(ctx)
        mutex->unlock();
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

    QMap<int, label_item> raw_labels;
    fz_var(raw_labels);
    fz_try(ctx)
    {
        pdf_obj *labels = pdf_dict_gets(ctx, pdf_dict_gets(ctx, pdf_trailer(ctx, doc), "Root"), "PageLabels");
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

    pageLabels.clear();

    // Check if anything was found.
    if (raw_labels.isEmpty())
        return;

    // Convert raw labels to something useful.
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
            // Generate range of page labels.
            int i = it.key();
            const int next_num = (std::next(it) == raw_labels.cend()) ? number_of_pages : std::next(it).key();
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
    if (!pages.value(pagenumber) || resolution <= 0. || !ctx)
        return;

    // This is almost completely copied from a mupdf example.

    mutex->lock();
    // sender gets a references to context.
    *context = ctx;
    // Get a page (must be done in the main thread!).
    // This causes warnings if the page contains multimedia content.
    *bbox = pdf_bound_page(ctx, pages[pagenumber]);
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
    fz_var(pages[pagenumber]);
    fz_try(ctx)
    {
        // Prepare a display list for a drawing device.
        // The list (and not the page itself) will then be used to render the page.
        *list = fz_new_display_list(ctx, *bbox);
        // Use a fitz device to fill the list with the content of the page.
        dev = fz_new_list_device(ctx, *list);
        // One could use the "pdf_run_page_contents" function here instead to hide annotations.
        // But there exist PDFs in which images are not rendered by that function.
        pdf_run_page(ctx, pages[pagenumber], dev, fz_scale(resolution, resolution), NULL);
    }
    fz_always(ctx)
    {
        fz_drop_device(ctx, dev);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        fz_drop_display_list(ctx, *list);
        *list = NULL;
    }
}

const PdfDocument::SlideTransition MuPdfDocument::transition(const int page) const
{
    SlideTransition trans;
    if (!pages.value(page) || !ctx)
        return trans;

    mutex->lock();
    fz_transition doc_trans;
    float duration;
    fz_var(doc_trans);
    fz_var(duration);
    fz_var(pages[page]);
    fz_try(ctx)
        pdf_page_presentation(ctx, pages[page], &doc_trans, &duration);
    fz_catch(ctx)
    {
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
            pdf_obj *transdict = pdf_dict_get(ctx, pages[page]->obj, PDF_NAME(Trans));
            if (pdf_dict_get_bool(ctx, transdict, PDF_NAME(B)))
                trans.type = SlideTransition::FlyRectangle;
            pdf_obj *ss_obj = pdf_dict_gets(ctx, transdict, "SS");
            if (ss_obj)
                trans.scale = pdf_to_real(ctx, ss_obj);
        }
        fz_catch(ctx)
             warn_msg("failed to completely load slide transition" << fz_caught_message(ctx));
    }

    mutex->unlock();
    return trans;
}

const PdfDocument::PdfLink MuPdfDocument::linkAt(const int page, const QPointF &position) const
{
    PdfLink result;
    if (!pages.value(page) || !ctx || !doc)
        return result;

    mutex->lock();
    fz_link *clink = NULL;
    fz_var(clink);
    fz_var(result);
    fz_try(ctx)
    {
        clink = pdf_load_links(ctx, pages[page]);
        for (fz_link* link = clink; link != NULL; link = link->next)
        {
            if (link->rect.x0 <= position.x() && link->rect.x1 >= position.x() && link->rect.y0 <= position.y() && link->rect.y1 >= position.y())
            {
                const QRectF rect = QRectF(link->rect.x0, link->rect.y0, link->rect.x1 - link->rect.x0, link->rect.y1 - link->rect.y0).normalized();
                if (link->uri == NULL)
                    result = {PdfLink::NoLink, "", rect};
                else if (link->uri[0] == '#')
                {
                    // Internal navigation link
                    float x, y;
                    const int location = pdf_resolve_link(ctx, doc, link->uri, &x, &y);
                    result = {location, "", rect};
                }
                else
                {
                    debug_msg(DebugRendering, "Unsupported link" << link->uri);
                    result = {PdfLink::NoLink, "", rect};
                }
                debug_verbose(DebugRendering, "Link to" << link->uri);
                break;
            }
        }
    }
    fz_always(ctx)
    {
        fz_drop_link(ctx, clink);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        warn_msg("Error while loading link" << fz_caught_message(ctx));
    }
    return result;
}

QList<PdfDocument::MediaAnnotation> *MuPdfDocument::annotations(const int page) const
{
    if (!pages.value(page) || !ctx)
        return NULL;
    QList<MediaAnnotation>* list = NULL;
    mutex->lock();
    fz_var(list);
    fz_try(ctx)
    {
        for (pdf_annot *annot = pdf_first_annot(ctx, pages[page]); annot != NULL; annot = pdf_next_annot(ctx, annot))
        {
            debug_verbose(DebugMedia, "PDF annotation:" << pdf_annot_type(ctx, annot) << page);
            switch (pdf_annot_type(ctx, annot))
            {
            case PDF_ANNOT_MOVIE:
            {
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 19)
                pdf_obj *media_obj = pdf_dict_gets(ctx, pdf_annot_obj(ctx, annot), "Movie");
#else
                pdf_obj *media_obj = pdf_dict_gets(ctx, annot->obj, "Movie");
#endif
                if (!media_obj)
                {
                    qWarning() << "Error while reading movie annotation";
                    break;
                }
                const QFileInfo fileinfo(pdf_dict_get_text_string(ctx, media_obj, PDF_NAME(F)));
                //pdf_drop_obj(ctx, media_obj);
                if (!fileinfo.exists())
                    continue;
                fz_rect bound = pdf_bound_annot(ctx, annot);
                if (list == NULL)
                    list = new QList<MediaAnnotation>();
                list->append(MediaAnnotation(
                            QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                            true,
                            QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0)
                        ));
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 19)
                pdf_obj *activation_obj = pdf_dict_get(ctx, pdf_annot_obj(ctx, annot), PDF_NAME(A));
#else
                pdf_obj *activation_obj = pdf_dict_get(ctx, annot->obj, PDF_NAME(A));
#endif
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
            case PDF_ANNOT_SOUND:
            {
                // TODO: embedded sounds
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 19)
                pdf_obj *media_obj = pdf_dict_gets(ctx, pdf_annot_obj(ctx, annot), "Sound");
#else
                pdf_obj *media_obj = pdf_dict_gets(ctx, annot->obj, "Sound");
#endif
                if (!media_obj)
                {
                    warn_msg("Error while reading sound annotation");
                    break;
                }
                const QFileInfo fileinfo(pdf_dict_get_text_string(ctx, media_obj, PDF_NAME(F)));
                //pdf_drop_obj(ctx, media_obj);
                if (!fileinfo.exists())
                {
                    warn_msg("Failed to load sound object: file not found or unsupported embedded sound");
                    continue;
                }
                fz_rect bound = pdf_bound_annot(ctx, annot);
                if (list == NULL)
                    list = new QList<MediaAnnotation>();
                list->append(MediaAnnotation(
                            QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                            false,
                            QRectF(bound.x0, bound.y0, bound.x1-bound.x0, bound.y1-bound.y0)
                        ));
                break;
            }
#if (FZ_VERSION_MAJOR >= 1) && (FZ_VERSION_MINOR >= 18)
            case PDF_ANNOT_RICH_MEDIA:
                // TODO: check what that does
                warn_msg("Unsupported media type: rich media");
                break;
#endif
            default:
                break;
            }
        }
    }
    fz_always(ctx)
    {
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        warn_msg("Error while searching annotations:" << fz_caught_message(ctx));
    }

    // TODO: Sounds included as links or actions are not supported in MuPDF.
    /*
    fz_link *clink = NULL;
    mutex->lock();
    fz_var(clink);
    fz_try(ctx)
    {
        clink = pdf_load_links(ctx, pages[page]);
        for (fz_link* link = clink; link != NULL; link = link->next)
        {
            QRectF rect(link->rect.x0, link->rect.y0, link->rect.x1-link->rect.x0, link->rect.y1-link->rect.y0);
            debug_verbose(DebugMedia, link->uri << rect << page);
        }
    }
    fz_always(ctx)
    {
        fz_drop_link(ctx, clink);
        mutex->unlock();
    }
    fz_catch(ctx)
    {
        warn_msg("Error while loading links" << fz_caught_message(ctx));
    }
    */

    return list;
}

bool MuPdfDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || !ctx || !pages.value(0))
        return flexible_page_sizes;

    flexible_page_sizes = 0;
    mutex->lock();
    const fz_rect ref_bbox = pdf_bound_page(ctx, pages[0]);
    fz_rect bbox;
    for (auto page : qAsConst(pages))
    {
        bbox = pdf_bound_page(ctx, page);
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
    if (!ctx || !doc)
        return;

    // TODO: a huge outline will probably lead to a crash of the program.
    outline.clear();
    outline.append({"", -1, 1});
    mutex->lock();

    fz_outline *root;
    fz_var(root);
    fz_var(outline);
    fz_try(ctx)
    {
        root = pdf_load_outline(ctx, doc);
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
    if ((preferences()->debug_level & (DebugRendering|DebugVerbose)) == (DebugRendering|DebugVerbose))
        for (int i=0; i<outline.length(); i++)
            qDebug() << DebugRendering << i << outline[i].page << outline[i].next << outline[i].title;
#endif
}

qreal MuPdfDocument::duration(const int page) const noexcept
{
    if (!pages.value(page) || !ctx)
        return -1.;
    mutex->lock();
    qreal duration;
    fz_var(duration);
    fz_try(ctx)
    {
        pdf_obj *obj = pdf_dict_get(ctx, pages[page]->obj, PDF_NAME(Dur));
        duration = obj ? pdf_to_real(ctx, obj) : -1.;
    }
    fz_always(ctx)
        mutex->unlock();
    fz_catch(ctx)
        duration = -1.;
    return duration;
}
