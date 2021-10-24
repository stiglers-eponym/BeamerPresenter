#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <set>
#include "src/enumerates.h"
#if (QT_VERSION_MAJOR == 6)
#include <poppler/qt6/poppler-qt6.h>
#elif (QT_VERSION_MAJOR == 5)
#include <poppler/qt5/poppler-qt5.h>
#include <memory>
#endif
#include "src/rendering/pdfdocument.h"

class PngPixmap;

class PopplerDocument : public PdfDocument
{
    /// Poppler document representing the PDF.
    std::unique_ptr<Poppler::Document> doc = NULL;

    /// Map page numbers to labels: Only the first page number with a new label
    /// is listed here.
    QMap<int, QString> pageLabels;

    /// populate pageLabels.
    void loadPageLabels();

public:
    /// Constructor: calls loadDocument().
    PopplerDocument(const QString &filename);

    /// Destructor: trivial
    ~PopplerDocument() noexcept override {}

    /// Render page to QPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;

    /// Render page to PngPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override final;

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
    {return pageLabels.size() == doc->numPages() ? QList<int>() : pageLabels.keys();}

    /// Link at given position (in point = inch/72).
    const PdfLink linkAt(const int page, const QPointF &position) const override;

    /// List all video annotations on given page. Returns NULL if list is empty.
    virtual QList<MediaAnnotation>* annotations(const int page) const override;

    /// Slide transition when reaching the given page.
    const SlideTransition transition(const int page) const override;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;

    /// Duration of given page in secons. Default value is -1 is interpreted as infinity.
    qreal duration(const int page) const noexcept override;
};

static const QMap<Poppler::SoundObject::SoundEncoding, PdfDocument::EmbeddedMedia::Encoding> convert_sound_encoding = {
    {Poppler::SoundObject::Raw, PdfDocument::EmbeddedMedia::SoundEncodingRaw},
    {Poppler::SoundObject::ALaw, PdfDocument::EmbeddedMedia::SoundEncodingALaw},
    {Poppler::SoundObject::muLaw, PdfDocument::EmbeddedMedia::SoundEncodingMuLaw},
    {Poppler::SoundObject::Signed, PdfDocument::EmbeddedMedia::SoundEncodingSigned},
};

#endif // POPPLERDOCUMENT_H
