#ifndef POPPLERRENDERER_H
#define POPPLERRENDERER_H

#include <QBuffer>
#include <QDebug>
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/popplerdocument.h"

class PopplerRenderer : public AbstractRenderer
{
    /// Poppler PDF document.
    const PopplerDocument *doc;

public:
    /// Constructor: Only initializes doc and page_part.
    /// This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    PopplerRenderer(const PopplerDocument *document, const PagePart part = FullPage) : AbstractRenderer(part), doc(document) {};

    /// Trivial destructor.
    ~PopplerRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override
    {return doc ? doc->getPixmap(page, resolution, page_part) : QPixmap();}

    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override
    {return doc ? doc->getPng(page, resolution, page_part) : nullptr;}

    /// Check whether doc is valid.
    bool isValid() const override
    {return doc && doc->isValid();}
};

#endif // POPPLERRENDERER_H
