#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <poppler/qt5/poppler-qt5.h>
#include <QFileInfo>
#include <QInputDialog>
#include <set>
#include "src/rendering/pdfdocument.h"
#include "src/enumerates.h"

/*
static const QMap<Poppler::PageTransition::Type, SlideTransition::Type> mapTransitionTypes
{
    {Poppler::PageTransition::Replace, SlideTransition::Replace},
    {Poppler::PageTransition::Split, SlideTransition::Split},
    {Poppler::PageTransition::Blinds, SlideTransition::Blinds},
    {Poppler::PageTransition::Box, SlideTransition::Box},
    {Poppler::PageTransition::Wipe, SlideTransition::Wipe},
    {Poppler::PageTransition::Dissolve, SlideTransition::Dissolve},
    {Poppler::PageTransition::Glitter, SlideTransition::Glitter},
    {Poppler::PageTransition::Fly, SlideTransition::Fly},
    {Poppler::PageTransition::Push, SlideTransition::Push},
    {Poppler::PageTransition::Cover, SlideTransition::Cover},
    {Poppler::PageTransition::Uncover, SlideTransition::Uncover},
    {Poppler::PageTransition::Fade, SlideTransition::Fade},
};
*/

class PopplerDocument : public PdfDocument
{
    /// Poppler document representing the PDF.
    const Poppler::Document *doc {NULL};

    /// Lookup table for page labels: set of page indices, at which the page
    /// label changes. This is left empty if every page starts with a new
    /// label.
    std::set<int> overlay_slide_indices;

    /// Generate overlay_slide_indices.
    void populateOverlaySlidesSet();

public:
    /// Constructor: calls loadDocument().
    PopplerDocument(const QString &filename);

    /// Destructor: deletes doc.
    ~PopplerDocument() override
    {delete doc;}

    /// Render page to QPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;

    /// Render page to PngPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override;

    /// Size of page in points (inch/72). Empty if page is invalid.
    const QSizeF pageSize(const int page) const override;

    /// Number of pages (0 if doc is null).
    int numberOfPages() const override
    {return doc ? doc->numPages() : 0;}

    /// Check whether a file has been loaded successfully.
    bool isValid() const override
    {return doc && !doc->isLocked();}

    /// Load the PDF outline, fill PdfDocument::outline.
    void loadOutline() override;

    /// Page label of given page index. (Empty string if page is invalid.)
    const QString pageLabel(const int page) const override;

    /// Label of page with given index.
    int pageIndex(const QString &page) const override;

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    virtual QList<int> overlayIndices() const noexcept override
    {return QList<int>(overlay_slide_indices.cbegin(), overlay_slide_indices.cend());}

    /// Link at given position (in point = inch/72).
    const PdfLink linkAt(const int page, const QPointF &position) const override;

    /// Annotation at given position (in point = inch/72)
    virtual const VideoAnnotation annotationAt(const int page, const QPointF &position) const override;

    /// List all video annotations on given page. Returns NULL if list is empty.
    virtual QList<VideoAnnotation>* annotations(const int page) const override;

    /// Slide transition when reaching the given page.
    const SlideTransition transition(const int page) const override;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;
};

#endif // POPPLERDOCUMENT_H
