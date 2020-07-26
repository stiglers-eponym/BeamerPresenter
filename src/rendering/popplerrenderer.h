#ifndef POPPLERRENDERER_H
#define POPPLERRENDERER_H

#include <QBuffer>
#include <QDebug>
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/popplerdocument.h"

class PopplerRenderer : public AbstractRenderer
{
    const PopplerDocument *doc;

public:
    /// Create renderer. This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    PopplerRenderer(const PopplerDocument *document, const PagePart part = FullPage) : AbstractRenderer(part), doc(document) {};
    ~PopplerRenderer() override {}
    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;
    bool isValid() const override {return doc != nullptr && doc->isValid();}
};

#endif // POPPLERRENDERER_H
