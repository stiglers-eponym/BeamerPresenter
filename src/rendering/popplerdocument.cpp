#include "src/rendering/popplerdocument.h"
#include "src/rendering/pngpixmap.h"
#include "src/preferences.h"
#include <QBuffer>
#include <QFileInfo>
#include <QInputDialog>

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
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    return docpage ? docpage->label() : "";
}

int PopplerDocument::pageIndex(const QString &page) const
{
    const std::unique_ptr<Poppler::Page> pageptr = doc->page(page);
    const int index = pageptr ? pageptr->index() : -1;
    return index;
}

const QSizeF PopplerDocument::pageSize(const int page) const
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    return docpage ? docpage->pageSizeF() : QSizeF();
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
    std::unique_ptr<Poppler::Document> newdoc = Poppler::Document::load(path);
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
        // Check if a password was entered and try to unlock document.
        // Only user password is required, since we only read the document.
        if (!ok || password.isEmpty() || newdoc->unlock(QByteArray(), password.toUtf8()))
        {
            qCritical() << "No or invalid password provided for locked document";
            newdoc = NULL;
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
    if (newdoc != NULL)
        doc.swap(newdoc);
    flexible_page_sizes = -1;

    populateOverlaySlidesSet();
    return true;
}

const QPixmap PopplerDocument::getPixmap(const int page, const qreal resolution, const PagePart page_part) const
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    if (resolution <= 0 || !docpage)
    {
        qWarning() << "Tried to render invalid page or invalid resolution" << page;
        return QPixmap();
    }
    const QImage image = docpage->renderToImage(72.*resolution, 72.*resolution);
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
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    if (resolution <= 0 || !docpage)
    {
        qWarning() << "Tried to render invalid page or invalid resolution" << page;
        return NULL;
    }
    QImage image = docpage->renderToImage(72.*resolution, 72.*resolution);
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
        const std::unique_ptr<Poppler::Page> page = doc->page(i);
        if (page && label != page->label())
        {
            if (!useful && !overlay_slide_indices.empty() && *overlay_slide_indices.crbegin() != i-1)
                useful = true;
            overlay_slide_indices.insert(i);
            label = page->label();
        }
    }
    if (!useful)
    {
        // All slides have their own labels.
        // It does not make any sence to store this list (which ist 0,1,2,...).
        overlay_slide_indices.clear();
    }
}

const PdfDocument::PdfLink PopplerDocument::linkAt(const int page, const QPointF &position) const
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    if (!docpage)
        return PdfLink();
    const QSizeF pageSize = docpage->pageSizeF();
    const QPointF relpos = {position.x()/pageSize.width(), position.y()/pageSize.height()};
    const auto links = docpage->links();
    for (auto it = links.cbegin(); it != links.cend(); ++it)
    {
        if ((*it)->linkArea().normalized().contains(relpos))
        {
            QRectF rect = (*it)->linkArea().normalized();
            rect.moveTop(rect.top()*pageSize.height());
            rect.moveLeft(rect.left()*pageSize.width());
            rect.setSize({pageSize.width() * rect.width(), pageSize.height() * rect.height()});
            switch ((*it)->linkType())
            {
            case Poppler::Link::LinkType::Goto: {
                const Poppler::LinkGoto *gotolink = static_cast<Poppler::LinkGoto*>(it->get());
                return {gotolink->destination().pageNumber() - 1, "", rect};
            }
            default:
                debug_msg(DebugRendering) << "Unsupported link" << (*it)->linkType();
                return {PdfLink::NoLink, "", rect};
            }
        }
    }
    return PdfLink();
}

QList<PdfDocument::MediaAnnotation> *PopplerDocument::annotations(const int page) const
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    if (!docpage)
        return NULL;
    debug_verbose(DebugMedia) << "Found" << docpage->annotations().size() << "annotations on page" << page;
    const auto annotations = docpage->annotations({Poppler::Annotation::AMovie, Poppler::Annotation::ASound, Poppler::Annotation::ARichMedia});
    const auto links = docpage->links();
    debug_verbose(DebugMedia) << "Found" << links.size() << "links on page" << page;
    if (annotations.empty() && links.empty())
        return NULL;
    const QSizeF pageSize = docpage->pageSizeF();
    QList<MediaAnnotation> *list = new QList<MediaAnnotation>;
    for (auto it = annotations.cbegin(); it != annotations.cend(); ++it)
    {
        // TODO: try to find better way of handling URLs
        switch ((*it)->subType())
        {
        case Poppler::Annotation::AMovie:
        {
            const Poppler::MovieObject *movie = static_cast<Poppler::MovieAnnotation*>(it->get())->movie();
            QFileInfo fileinfo(movie->url());
            if (fileinfo.exists())
            {
                list->append(MediaAnnotation(
                            QUrl::fromLocalFile(fileinfo.absoluteFilePath()),
                            true,
                            QRectF(pageSize.width()*(*it)->boundary().x(), pageSize.height()*(*it)->boundary().y(), pageSize.width()*(*it)->boundary().width(), pageSize.height()*(*it)->boundary().height())
                ));
                debug_verbose(DebugMedia) << "Found video annotation:" << fileinfo.filePath() << "on page" << page;
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
            break;
        }
        case Poppler::Annotation::ASound:
        {
            const Poppler::SoundObject *sound = static_cast<Poppler::SoundAnnotation*>(it->get())->sound();
            QRectF area = (*it)->boundary();
            area = {pageSize.width()*area.x(), pageSize.height()*area.y(), pageSize.width()*area.width(), pageSize.height()*area.height()};
            switch (sound->soundType())
            {
            case Poppler::SoundObject::Embedded:
                debug_verbose(DebugMedia) << "Found sound annotation: embedded on page" << page;
                if (!sound->data().isEmpty())
                {
                    EmbeddedMedia media(sound->data(), sound->samplingRate(), area);
                    media.channels = sound->channels();
                    media.bit_per_sample = sound->bitsPerSample();
                    media.encoding = convert_sound_encoding.value(sound->soundEncoding(), PdfDocument::EmbeddedMedia::SoundEncodingRaw);
                    list->append(media);
                }
                break;
            case Poppler::SoundObject::External:
            {
                QFileInfo fileinfo(sound->url());
                debug_verbose(DebugMedia) << "Found sound annotation:" << fileinfo.filePath() << "on page" << page;
                if (fileinfo.exists())
                    list->append(MediaAnnotation(QUrl::fromLocalFile(fileinfo.absoluteFilePath()), false, area));
                break;
            }
            }
            break;
        }
        case Poppler::Annotation::ARichMedia:
            warn_msg << "Unsupported media type: rich media";
            break;
        default:
            break;
        }
    }
    for (auto it = links.cbegin(); it != links.cend(); ++it)
    {
        debug_verbose(DebugMedia) << "Link of type" << (*it)->linkType() << (*it)->linkArea() << page;
        if ((*it)->linkType() == Poppler::Link::Sound)
        {
            const Poppler::LinkSound *link = static_cast<Poppler::LinkSound*>(it->get());
            QRectF area = link->linkArea();
            area = {pageSize.width()*area.x(), pageSize.height()*area.y(), pageSize.width()*area.width(), pageSize.height()*area.height()};
            switch (link->sound()->soundType())
            {
            case Poppler::SoundObject::Embedded:
                debug_verbose(DebugMedia) << "Found sound link: embedded on page" << page;
                if (!link->sound()->data().isEmpty())
                {
                    EmbeddedMedia media(link->sound()->data(), link->sound()->samplingRate(), area);
                    media.channels = link->sound()->channels();
                    media.bit_per_sample = link->sound()->bitsPerSample();
                    media.encoding = convert_sound_encoding.value(link->sound()->soundEncoding(), PdfDocument::EmbeddedMedia::SoundEncodingRaw);
                    media.volume = link->volume();
                    media.mode = link->repeat() ? PdfDocument::MediaAnnotation::Once : PdfDocument::MediaAnnotation::Repeat;
                    list->append(media);
                }
                break;
            case Poppler::SoundObject::External:
            {
                QFileInfo fileinfo(link->sound()->url());
                debug_verbose(DebugMedia) << "Found sound link:" << fileinfo.filePath() << "on page" << page;
                if (fileinfo.exists())
                    list->append(MediaAnnotation(QUrl::fromLocalFile(fileinfo.absoluteFilePath()), false, area));
                break;
            }
            }
            link->sound()->data();
        }
    }
    if (list->isEmpty())
    {
        delete list;
        return NULL;
    }
    return list;
}

const PdfDocument::SlideTransition PopplerDocument::transition(const int page) const
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    if (!docpage)
        return SlideTransition();
    const Poppler::PageTransition *doc_trans = docpage->transition();
    if (!doc_trans)
        return SlideTransition();

    SlideTransition trans;
    trans.type = static_cast<SlideTransition::Type>(doc_trans->type());
    trans.duration = doc_trans->durationReal();
    trans.properties = (doc_trans->alignment() == Poppler::PageTransition::Vertical ? SlideTransition::Vertical : 0)
            | (doc_trans->direction() == Poppler::PageTransition::Outward ? SlideTransition::Outwards : 0);
    trans.angle = doc_trans->angle();
    if (trans.type == SlideTransition::Fly)
    {
        if (doc_trans->isRectangular())
            trans.type = SlideTransition::FlyRectangle;
        trans.scale = doc_trans->scale();
    }
    return trans;
}

bool PopplerDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || doc == NULL)
        return flexible_page_sizes;
    const QSizeF ref_size = doc->page(0)->pageSizeF();
    for (int page=1; page<doc->numPages(); page++)
    {
        const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
        if (docpage && docpage->pageSizeF() != ref_size)
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
    if (root.isEmpty())
        return;
    // dangerous anonymous recursion
    auto fill_outline = [&](const Poppler::OutlineItem &entry, const char sign, auto& function) -> void
    {
        const int idx = outline.length();
        outline.append({entry.name(), entry.destination()->pageNumber() - 1, -1});
        const auto &children = entry.children();
        if (!children.isEmpty())
        {
            const auto end = children.cend() - 1;
            auto it = children.cbegin();
            while (it != end)
                function(*it++, 1, function);
            function(*it, -1, function);
        }
        outline[idx].next = sign*outline.length();
    };
    const auto end = root.cend() - 1;
    auto it = root.cbegin();
    while (it != end)
        fill_outline(*it++, 1, fill_outline);
    fill_outline(*it, -1, fill_outline);
#ifdef QT_DEBUG
    if ((preferences()->debug_level & (DebugRendering|DebugVerbose)) == (DebugRendering|DebugVerbose))
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

qreal PopplerDocument::duration(const int page) const noexcept
{
    const std::unique_ptr<Poppler::Page> docpage = doc->page(page);
    return docpage ? docpage->duration() : -1.;
}
