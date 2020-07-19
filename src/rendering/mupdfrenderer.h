#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <QDebug>
#include <QObject>
#include <QMetaType>
#include "src/rendering/mupdfdocument.h"
#include "src/rendering/abstractrenderer.h"

/// Renderer using the MuPDF backend. This renderer requires that the PDF
/// document has been loaded by MuPDF (and not Poppler).
class MuPdfRenderer : public AbstractRenderer
{
    const MuPdfDocument *doc;

public:
    MuPdfRenderer(const MuPdfDocument *doc) : AbstractRenderer(), doc(doc) {}
    ~MuPdfRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// In the current implementation this is always valid.
    bool isValid() const override {return doc != nullptr && doc->isValid();}
};

#endif // MUPDFRENDERER_H
