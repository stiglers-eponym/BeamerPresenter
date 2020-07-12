#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <poppler/qt5/poppler-qt5.h>
#include <QFileInfo>
#include <QInputDialog>
#include "src/rendering/pdfdocument.h"

class PopplerDocument : public PdfDocument
{
    Poppler::Document *doc = nullptr;
public:
    PopplerDocument(const QString &filename);
    ~PopplerDocument() override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const QPixmap getPixmap(const int page, const qreal resolution) const override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const PngPixmap* getPng(const int page, const qreal resolution) const override;
    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override;
    /// Size of page in points (72*inch).
    const QSizeF pageSize(const int page) const override;
    int numberOfPages() const override;
    bool isValid() const override;
    const QString label(const int page) const override;
};

#endif // POPPLERDOCUMENT_H
