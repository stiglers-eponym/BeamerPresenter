#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include "src/rendering/mupdfdocument.h"
#include "src/rendering/abstractrenderer.h"

#ifndef FZ_VERSION_MAJOR
#define FZ_VERSION_MAJOR 0
#endif
#ifndef FZ_VERSION_MINOR
#define FZ_VERSION_MINOR 0
#endif

/// Renderer using the MuPDF engine. This renderer requires that the PDF
/// document has been loaded by MuPDF (and not Poppler).
/// The rendering requires preparation done by const functions in the
/// MuPdfDocument (which in general lives in a differen thread).
class MuPdfRenderer : public AbstractRenderer
{
    const MuPdfDocument *doc;

    /// Helper function for rendering functions.
    fz_pixmap *renderFzPixmap(const int page, const qreal resolution, fz_context *&ctx) const;

public:
    /// Constructor: only initializes doc and page_part.
    MuPdfRenderer(const MuPdfDocument *doc, const PagePart part = FullPage) : AbstractRenderer(part), doc(doc) {}

    /// Trivial destructor.
    ~MuPdfRenderer() override {}

    /// Render page to a QPixmap. Resolution is given in pixels per point
    /// (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;

    /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// In the current implementation this is always valid.
    bool isValid() const override
    {return doc != NULL && doc->isValid();}
};

#endif // MUPDFRENDERER_H
