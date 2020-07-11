#ifndef POPPLERRENDERER_H
#define POPPLERRENDERER_H

#include <poppler/qt5/poppler-qt5.h>
#include <QBuffer>
#include <QDebug>
#include "src/rendering/abstractrenderer.h"

class PopplerRenderer : public AbstractRenderer
{
    const Poppler::Document *doc;

public:
    /// Create renderer. This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    PopplerRenderer(const Poppler::Document *document) : doc(document) {};
    ~PopplerRenderer() override {}
    /// Render page to a QPixmap.
    /// Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in dpi.
    const QByteArray * renderPng(const int page, const qreal resolution) const override;
    bool isValid() const override {return doc != nullptr && !doc->isLocked();}
};

#endif // POPPLERRENDERER_H
