#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "src/enumerates.h"

class QPixmap;
class PngPixmap;

/// Abstract rendering class. Instances of implementing classes should be save
/// to use outside the main thread.
class AbstractRenderer
{
protected:
    /// Each page part of a document is associated to a single renderer.
    const PagePart page_part;

public:
    /// Type of the renderer.
    enum Renderer {
#ifdef INCLUDE_POPPLER
        Poppler = 0,
#endif
#ifdef INCLUDE_MUPDF
        MuPDF = 1,
#endif
        ExternalRenderer = 2,
    };

    /// Constructor: only initializes page_part.
    AbstractRenderer(const PagePart part = FullPage) : page_part(part) {};

    /// Trivial virtual destructor.
    virtual ~AbstractRenderer() {};

    /// get page_part;
    PagePart pagePart() const {return page_part;}

    /// Render page to a QPixmap. Resolution is given in pixels per point
    /// (dpi/72).
    virtual const QPixmap renderPixmap(const int page, const qreal resolution) const = 0;

    /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    virtual const PngPixmap *renderPng(const int page, const qreal resolution) const = 0;

    /// Check if renderer is valid and can in principle render pages.
    virtual bool isValid() const = 0;
};

#endif // ABSTRACTRENDERER_H
