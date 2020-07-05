#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <QString>
#include <QDebug>
#include <mupdf/fitz.h>
#include "abstractrenderer.h"

class MuPdfRenderer : public AbstractRenderer
{
    fz_context *context;
    fz_document *doc;
public:
    MuPdfRenderer(const QString &filename);
    ~MuPdfRenderer();
    /// Render page to a QPixmap.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    const QByteArray * renderPng(const int page, const qreal resolution) const override;
};

#endif // MUPDFRENDERER_H
