#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "src/enumerates.h"
#include "src/rendering/pngpixmap.h"
#include <QPixmap>
#include <QByteArray>

/// Abstract rendering class. Instances of implementing classes should be save
/// to use outside the main thread.
class AbstractRenderer
{
protected:
    const PagePart page_part;
public:
    enum Renderer {
#ifdef INCLUDE_POPPLER
        Poppler = 0,
#endif
#ifdef INCLUDE_MUPDF
        MuPDF = 1,
#endif
        ExternalRenderer = 2,
    };

    AbstractRenderer(const PagePart part = FullPage) : page_part(part) {};
    PagePart pagePart() const {return page_part;}
    virtual ~AbstractRenderer() {};
    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    virtual const QPixmap renderPixmap(const int page, const qreal resolution) const = 0;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    virtual const PngPixmap * renderPng(const int page, const qreal resolution) const = 0;
    virtual bool isValid() const = 0;
};

#endif // ABSTRACTRENDERER_H
