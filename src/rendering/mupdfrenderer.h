#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <QDebug>
#include <QObject>
#include <QMetaType>
#include "src/rendering/mupdfdocument.h"
#include "src/rendering/abstractrenderer.h"

/// Renderer using the MuPDF backend. This renderer requires that the PDF
/// document has been loaded by MuPDF (and not Poppler). It achieves thread
/// savety by using Qt's signaling system and must therefore be a QObject.
/// The only connection to the fitz document (MuPDF's document) is provided
/// by a signal emitted by MuPdfRenderer to MuPdfDocument. The signal must
/// be connected whenever a new object is created.
class MuPdfRenderer : public QObject, public AbstractRenderer
{
    Q_OBJECT

public:
    /// After creation MuPdfRenderer::prepareRendering MUST be connected to
    /// MuPdfDocument::prepareRendering. If both object live in the same
    /// thread, the connection type must be Qt::DirecConnection. If they live
    /// in different threads, it must be Qt::BlockingQueuedConnection.
    MuPdfRenderer() : AbstractRenderer() {}
    ~MuPdfRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// In the current implementation this is always valid.
    bool isValid() const override {return true;}

signals:
    /// Let MuPdfDocument in the main thread prepare the rendering.
    /// It initializes the values at the given pointers.
    void prepareRendering(fz_context **ctx, fz_rect* bbox, fz_display_list **list, const int pagenumber, const qreal resolution) const;
};

#endif // MUPDFRENDERER_H
