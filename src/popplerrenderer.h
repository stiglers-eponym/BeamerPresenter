#ifndef POPPLERRENDERER_H
#define POPPLERRENDERER_H

#include <poppler-qt5.h>
#include <QBuffer>
#include <QDebug>
#include "src/abstractrenderer.h"

class PopplerRenderer : public AbstractRenderer
{
    const Poppler::Document *doc;

public:
    /// Create renderer. This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    PopplerRenderer(const Poppler::Document *document) : doc(document) {};
    /// Render page to a QPixmap.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    const QByteArray * renderPng(const int page, const qreal resolution) const override;
};

#endif // POPPLERRENDERER_H
