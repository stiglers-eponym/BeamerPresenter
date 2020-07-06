#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <QString>
#include <QDebug>
#include <mupdf/fitz.h>
#include "src/rendering/abstractrenderer.h"

class MuPdfRenderer : public AbstractRenderer
{
    fz_context *context;
    fz_document *doc;

public:
    MuPdfRenderer(const QString &filename);
    ~MuPdfRenderer();

    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const QByteArray * renderPng(const int page, const qreal resolution) const override;

    /// Check whether this is valid, i.e. has nontrivial context and document.
    bool isValid() const override;
};

#endif // MUPDFRENDERER_H
