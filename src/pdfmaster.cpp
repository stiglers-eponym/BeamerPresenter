#include "src/pdfmaster.h"
#ifdef INCLUDE_POPPLER
#include "src/rendering/popplerdocument.h"
#endif
#ifdef INCLUDE_MUPDF
#include "src/rendering/mupdfdocument.h"
#endif

PdfMaster::PdfMaster(const QString &filename)
{
    // Load the document
#ifdef INCLUDE_POPPLER
#ifdef INCLUDE_MUPDF
    switch (preferences().pdf_backend)
    {
    case PdfDocument::PopplerBackend:
        document = new PopplerDocument(filename);
        break;
    case PdfDocument::MuPdfBackend:
        document = new MuPdfDocument(filename);
        break;
    }
#else
    document = new PopplerDocument(filename);
#endif
#else
    document = new MuPdfDocument(filename);
#endif
    if (document == nullptr || !document->isValid())
        qFatal("Loading document failed");
    // Generate the lookup table of page labels.
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
    return document->pageSize(page_number);
}

bool PdfMaster::loadDocument()
{
    if (!document->loadDocument())
        return false;
    populateOverlaySlidesSet();
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
        return document->numberOfPages() - 1;
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
    for (int i=0; i<document->numberOfPages(); ++i)
    {
        if (label != document->label(i))
        {
            if (!useful && *overlay_slide_indices.cend() != i-1)
                useful = true;
            overlay_slide_indices.insert(i);
            label = document->label(i);
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
    case ReloadFiles:
        if (loadDocument())
            // TODO: implement update
            emit update();
        break;
    default:
        break;
    }
}
