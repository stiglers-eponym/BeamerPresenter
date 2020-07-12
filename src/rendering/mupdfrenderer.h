#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <QDebug>
#include "src/rendering/mupdfdocument.h"
#include "src/rendering/abstractrenderer.h"

// TODO: Use shared document and cloned contexts instead of one document per renderer.

class MuPdfRenderer : public AbstractRenderer
{
    const MuPdfDocument *doc;
    fz_context *context;

public:
    MuPdfRenderer(const MuPdfDocument *doc);
    ~MuPdfRenderer() override;

    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// Check whether this is valid, i.e. has nontrivial context and document.
    bool isValid() const override;
};

#endif // MUPDFRENDERER_H
