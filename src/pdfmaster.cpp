#include <src/pdfmaster.h>

PdfMaster::PdfMaster(const QString &filename) :
    pdfpath(filename)
{
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");
    // Generate the lookup table of page labels.
    if (overlay_slide_indices.empty())
        populateOverlaySlidesSet();
}

PdfMaster::~PdfMaster()
{
    qDeleteAll(paths);
    paths.clear();
    delete document;
}

const QSizeF PdfMaster::getPageSize(const int page_number) const
{
    Poppler::Page const *page = document->page(page_number);
    if (page == nullptr)
        return QSizeF();
    return page->pageSizeF();
}

bool PdfMaster::loadDocument()
{
    QFileInfo const fileinfo(pdfpath);

    // Check if the file exists.
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        qCritical() << "Given filename is not a file.";
        return false;
    }
    // Check if the file has changed since last (re)load
    if (document != nullptr && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    Poppler::Document * newdoc = Poppler::Document::load(pdfpath);
    if (newdoc == nullptr)
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
                    nullptr,
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
            newdoc = nullptr;
            qCritical() << "No password provided for locked document";
            return false;
        }
        // Try to unlock document.
        if (!newdoc->unlock(QByteArray(), password.toLatin1()))
        {
            delete newdoc;
            newdoc = nullptr;
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
    if (document == nullptr)
        document = newdoc;
    else
    {
        Poppler::Document * const olddoc = document;
        document = newdoc;
        delete olddoc;
    }
    return true;
}

void PdfMaster::updatePaths(const SlideView* sender)
{
    // TODO!
}

int PdfMaster::overlaysShifted(const int start, const int shift_overlay) const
{
    // Poppler functions for converting between labels and numbers seem to be
    // optimized for normal documents and are probably inefficient for handling
    // page numbers in presentations with overlays.
    // Use a lookup table.

    // Get the "number" part of shift_overlay by removing the "overlay" flags.
    int shift = shift_overlay & ~AnyOverlay;
    // Check whether the document has non-trivial page labels and shift has
    // non-trivial overlay flags.
    if (*overlay_slide_indices.cbegin() < 0 || shift == shift_overlay)
        return start + shift;
    // Find the beginning of next slide.
    std::set<int>::const_iterator it = overlay_slide_indices.upper_bound(start);
    // Shift the iterator according to shift.
    while (shift > 0 && it != overlay_slide_indices.cbegin())
    {
        --shift;
        ++it;
    }
    while (shift < 0 && it != overlay_slide_indices.cend())
    {
        ++shift;
        --it;
    }
    // Check if the iterator has reached the beginning or end of the set.
    if (it == overlay_slide_indices.cbegin())
        return 0;
    if (it == overlay_slide_indices.cend())
    {
        if ((shift & FirstOverlay) != 0)
            return *--it;
        return document->numPages() - 1;
    }
    // Return first or last overlay depending on overlay flags.
    if ((shift & FirstOverlay) != 0)
        return *--it;
    return *it - 1;
}

void PdfMaster::populateOverlaySlidesSet()
{
    // Check whether it makes sense to create the lookup table.
    bool useful = false;
    QString label = "\n\n\n\n";
    for (int i=0; i<document->numPages(); ++i)
    {
        if (label != document->page(i)->label())
        {
            if (!useful && *overlay_slide_indices.cend() != i-1)
                useful = true;
            overlay_slide_indices.insert(i);
            label = document->page(i)->label();
        }
    }
    if (!useful)
    {
        // All slides have their own labels.
        // It does not make any sence to store this list (which ist 0,1,2,...).
        // Indicate this by setting it to {-1}.
        overlay_slide_indices.clear();
        overlay_slide_indices.insert(-1);
    }
}

void PdfMaster::receiveAction(const Action action)
{
    switch (action)
    {
    case Reload:
        if (loadDocument())
            // TODO: implement update
            emit update();
        break;
    default:
        break;
    }
}
