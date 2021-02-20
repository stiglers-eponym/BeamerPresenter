#include "src/rendering/popplerdocument.h"

PopplerDocument::PopplerDocument(const QString &filename) :
    PdfDocument(filename)
{
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");

    debug_msg(DebugRendering) << "Loaded PDF document in Poppler";
}

const QString PopplerDocument::pageLabel(const int page) const
{
    const Poppler::Page * const pageptr = doc->page(page);
    return pageptr ? pageptr->label() : "";
}

int PopplerDocument::pageIndex(const QString &page) const
{
    const Poppler::Page * const pageptr = doc->page(page);
    return pageptr ? pageptr->index() : -1;
}

const QSizeF PopplerDocument::pageSize(const int page) const
{
    const Poppler::Page * const pageptr = doc->page(page);
    return pageptr ? pageptr->pageSizeF() : QSizeF();
}

bool PopplerDocument::loadDocument()
{
    // Check if the file exists.
    QFileInfo const fileinfo(path);
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        qCritical() << "Given filename is not a file.";
        return false;
    }
    // Check if the file has changed since last (re)load
    if (doc != NULL && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    Poppler::Document * newdoc = Poppler::Document::load(path);
    if (newdoc == NULL)
    {
        qCritical() << "Failed to load document.";
        return false;
    }

    // Try to unlock a locked document.
    if (newdoc->isLocked())
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
        if (!ok || password.isEmpty())
        {
            delete newdoc;
            newdoc = NULL;
            qCritical() << "No password provided for locked document";
            return false;
        }
        // Try to unlock document.
        if (!newdoc->unlock(QByteArray(), password.toLatin1()))
        {
            delete newdoc;
            newdoc = NULL;
            qCritical() << "Unlocking document failed: wrong password or bug.";
            return false;
        }
    }
    // Save the modification time.
    lastModified = fileinfo.lastModified();

    // Set rendering hints.
    newdoc->setRenderHint(Poppler::Document::TextAntialiasing);
    newdoc->setRenderHint(Poppler::Document::TextHinting);
    newdoc->setRenderHint(Poppler::Document::TextSlightHinting);
    newdoc->setRenderHint(Poppler::Document::Antialiasing);
    newdoc->setRenderHint(Poppler::Document::ThinLineShape);

    // Update document and delete old document.
    if (doc == NULL)
        doc = newdoc;
    else
    {
        const Poppler::Document * const olddoc = doc;
        doc = newdoc;
        delete olddoc;
    }
    flexible_page_sizes = -1;

    populateOverlaySlidesSet();
    return true;
}

const QPixmap PopplerDocument::getPixmap(const int page, const qreal resolution, const PagePart page_part) const
{
    if (resolution <= 0 || page < 0 || page >= doc->numPages())
        return QPixmap();
    const Poppler::Page * const popplerPage = doc->page(page);
    if (popplerPage == NULL)
    {
        qWarning() << "Tried to render invalid page" << page;
        return QPixmap();
    }
    const QImage image = popplerPage->renderToImage(72.*resolution, 72.*resolution);
    switch (page_part)
    {
    case LeftHalf:
        return QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
    case RightHalf:
        return QPixmap::fromImage(image.copy((image.width()+1)/2, 0, image.width()/2, image.height()));
    default:
        return QPixmap::fromImage(image);
    }
}

const PngPixmap * PopplerDocument::getPng(const int page, const qreal resolution, const PagePart page_part) const
{
    if (resolution <= 0 || page < 0)
        return NULL;
    const Poppler::Page * const popplerPage = doc->page(page);
    if (popplerPage == NULL)
    {
        qWarning() << "Tried to render invalid page" << page;
        return NULL;
    }
    QImage image = popplerPage->renderToImage(72.*resolution, 72.*resolution);
    if (image.isNull())
    {
        qWarning() << "Rendering page to image failed";
        return NULL;
    }
    switch (page_part)
    {
    case LeftHalf:
        image = image.copy(0, 0, image.width()/2, image.height());
        break;
    case RightHalf:
        image = image.copy((image.width()+1)/2, 0, image.width()/2, image.height());
        break;
    default:
        break;
    }
    QByteArray* const bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG"))
    {
        qWarning() << "Saving page as PNG image failed";
        delete bytes;
        return NULL;
    }
    return new PngPixmap(bytes, page, resolution);
}

int PopplerDocument::overlaysShifted(const int start, const int shift_overlay) const
{
    // Get the "number" part of shift_overlay by removing the "overlay" flags.
    int shift = shift_overlay >= 0 ? shift_overlay & ~ShiftOverlays::AnyOverlay : shift_overlay | ShiftOverlays::AnyOverlay;
    // Check whether the document has non-trivial page labels and shift has
    // non-trivial overlay flags.
    if (overlay_slide_indices.empty() || shift == shift_overlay)
        return start + shift;
    // Find the beginning of next slide.
    std::set<int>::const_iterator it = overlay_slide_indices.upper_bound(start);
    // Shift the iterator according to shift.
    while (shift > 0 && it != overlay_slide_indices.cend())
    {
        --shift;
        ++it;
    }
    while (shift < 0 && it != overlay_slide_indices.cbegin())
    {
        ++shift;
        --it;
    }
    // Check if the iterator has reached the beginning or end of the set.
    if (it == overlay_slide_indices.cbegin())
        return 0;
    if (it == overlay_slide_indices.cend())
    {
        if (shift_overlay & FirstOverlay)
            return *--it;
        return doc->numPages() - 1;
    }
    // Return first or last overlay depending on overlay flags.
    if (shift_overlay & FirstOverlay)
        return *--it;
    return *it - 1;
}

void PopplerDocument::populateOverlaySlidesSet()
{
    // Poppler functions for converting between labels and numbers seem to be
    // optimized for normal documents and are probably inefficient for handling
    // page numbers in presentations with overlays.
    // Use a lookup table.

    overlay_slide_indices.clear();
    // Check whether it makes sense to create the lookup table.
    bool useful = false;
    QString label = "\n\n\n\n";
    for (int i=0; i<doc->numPages(); i++)
    {
        if (label != doc->page(i)->label())
        {
            if (!useful && !overlay_slide_indices.empty() && *overlay_slide_indices.crbegin() != i-1)
                useful = true;
            overlay_slide_indices.insert(i);
            label = doc->page(i)->label();
        }
    }
    if (!useful)
    {
        // All slides have their own labels.
        // It does not make any sence to store this list (which ist 0,1,2,...).
        overlay_slide_indices.clear();
    }
}

const PdfLink PopplerDocument::linkAt(const int page, const QPointF &position) const
{
    const QSizeF pageSize = doc->page(page)->pageSizeF();
    const QPointF relpos = {position.x()/pageSize.width(), position.y()/pageSize.height()};
    for (const auto link : static_cast<const QList<Poppler::Link*>>(doc->page(page)->links()))
    {
        if (link->linkArea().normalized().contains(relpos))
        {
            QRectF rect = link->linkArea().normalized();
            rect.moveTop(rect.top()*pageSize.height());
            rect.moveLeft(rect.left()*pageSize.width());
            rect.setSize({pageSize.width() * rect.width(), pageSize.height() * rect.height()});
            switch (link->linkType())
            {
            case Poppler::Link::LinkType::Goto: {
                Poppler::LinkGoto *gotolink = static_cast<Poppler::LinkGoto*>(link);
                return {gotolink->destination().pageNumber()-1, "", rect};
            }
            default:
                debug_msg(DebugRendering) << "Unsupported link" << link->linkType();
                return {NoLink, "", rect};
            }
        }
    }
    return {NoLink, "", QRectF()};
}

const MediaAnnotation PopplerDocument::annotationAt(const int page, const QPointF &position) const
{
    const QSizeF pageSize = doc->page(page)->pageSizeF();
    const QPointF relpos = {position.x()/pageSize.width(), position.y()/pageSize.height()};
    for (const auto annotation : static_cast<const QList<Poppler::Annotation*>>(doc->page(page)->annotations({Poppler::Annotation::AMovie, Poppler::Annotation::ASound})))
    {
        if (annotation->boundary().contains(relpos))
        {
            Poppler::MovieObject *movie = static_cast<Poppler::MovieAnnotation*>(annotation)->movie();
            QFileInfo fileinfo(movie->url());
            if (!fileinfo.exists())
            {
                delete movie;
                continue;
            }
            MediaAnnotation videoAnnotation {
                        QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                        page,
                        annotation->subType() == Poppler::Annotation::AMovie ? MediaAnnotation::VideoAnnotation : MediaAnnotation::AudioAnnotation,
                        MediaAnnotation::Once,
                        {pageSize.width()*annotation->boundary().x(), pageSize.height()*annotation->boundary().y(), pageSize.width()*annotation->boundary().width(), pageSize.height()*annotation->boundary().height()}
            };
            switch (movie->playMode())
            {
            case Poppler::MovieObject::PlayOpen:
                videoAnnotation.mode = MediaAnnotation::Open;
                break;
            case Poppler::MovieObject::PlayPalindrome:
                videoAnnotation.mode = MediaAnnotation::Palindrome;
                break;
            case Poppler::MovieObject::PlayRepeat:
                videoAnnotation.mode = MediaAnnotation::Repeat;
                break;
            default:
                break;
            }
            delete movie;
            return videoAnnotation;
        }
    }
    return {QUrl(), page, MediaAnnotation::InvalidAnnotation, MediaAnnotation::Invalid, QRectF()};
}

QList<MediaAnnotation> *PopplerDocument::annotations(const int page) const
{
    const QList<Poppler::Annotation*> annotations = doc->page(page)->annotations({Poppler::Annotation::AMovie, Poppler::Annotation::ASound});
    if (annotations.isEmpty())
        return NULL;
    const QSizeF pageSize = doc->page(page)->pageSizeF();
    QList<MediaAnnotation> *list = new QList<MediaAnnotation>;
    for (const auto annotation : annotations)
    {
        Poppler::MovieObject *movie = static_cast<Poppler::MovieAnnotation*>(annotation)->movie();
        QFileInfo fileinfo(movie->url());
        if (fileinfo.exists())
        {
            list->append({
                        QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                        page,
                        annotation->subType() == Poppler::Annotation::AMovie ? MediaAnnotation::VideoAnnotation : MediaAnnotation::AudioAnnotation,
                        MediaAnnotation::Once,
                        {pageSize.width()*annotation->boundary().x(), pageSize.height()*annotation->boundary().y(), pageSize.width()*annotation->boundary().width(), pageSize.height()*annotation->boundary().height()}
            });
            switch (movie->playMode())
            {
            case Poppler::MovieObject::PlayOpen:
                list->last().mode = MediaAnnotation::Open;
                break;
            case Poppler::MovieObject::PlayPalindrome:
                list->last().mode = MediaAnnotation::Palindrome;
                break;
            case Poppler::MovieObject::PlayRepeat:
                list->last().mode = MediaAnnotation::Repeat;
                break;
            default:
                break;
            }
        }
        delete movie;
    }
    if (list->isEmpty())
    {
        delete list;
        return NULL;
    }
    return list;
}

const SlideTransition PopplerDocument::transition(const int page) const
{
    const Poppler::Page *doc_page = doc->page(page);
    if (!doc_page)
        return {};
    const Poppler::PageTransition *doc_trans = doc_page->transition();
    if (!doc_trans)
        return {};

    SlideTransition trans;
    //trans.type = mapTransitionTypes[doc_trans->type()];
    trans.type = static_cast<SlideTransition::Type>(doc_trans->type());
    trans.duration = doc_trans->durationReal();
    trans.angle = doc_trans->angle();
    if (trans.type == SlideTransition::Fly)
    {
        if (doc_trans->isRectangular())
            trans.type = SlideTransition::FlyRectangle;
        trans.scale = doc_trans->scale();
    }
    delete doc_trans;
    return trans;
}

bool PopplerDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || doc == NULL)
        return flexible_page_sizes;
    const QSizeF ref_size = doc->page(0)->pageSizeF();
    for (int page=1; page<doc->numPages(); page++)
    {
        if (doc->page(page)->pageSizeF() != ref_size)
        {
            flexible_page_sizes = 1;
            return 1;
        }
    }
    flexible_page_sizes = 0;
    return 0;
}

void PopplerDocument::loadOutline()
{
    // TODO: a huge outline will probably lead to a crash of the program.
    outline.clear();
    outline.append({"", -1, 1});
    const QVector<Poppler::OutlineItem> root = doc->outline();
    // dangerous anonymous recursion
    auto fill_outline = [&](const Poppler::OutlineItem &entry, auto& function) -> void
    {
        const int idx = outline.length();
        outline.append({entry.name(), entry.destination()->pageNumber() - 1, -1});
        for (const auto &child : static_cast<const QVector<Poppler::OutlineItem>>(entry.children()))
            function(child, function);
        outline[idx].next = outline.length();
        if (idx + 1 < outline.length())
            outline.last().next *= -1;
    };
    outline.last().next = -outline.length();
    for (const auto &child : root)
        fill_outline(child, fill_outline);
#ifdef QT_DEBUG
    if ((preferences()->log_level & (DebugRendering|DebugVerbose)) == (DebugRendering|DebugVerbose))
        for (int i=0; i<outline.length(); i++)
            qDebug() << DebugRendering << i << outline[i].page << outline[i].next << outline[i].title;
#endif
}

QList<int> PopplerDocument::overlayIndices() const noexcept
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    return QList<int>(overlay_slide_indices.cbegin(), overlay_slide_indices.cend());
#else
    QList<int> list;
    for (auto item : overlay_slide_indices)
        list.append(item);
    return list;
#endif
}
