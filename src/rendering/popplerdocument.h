#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <poppler/qt5/poppler-qt5.h>
#include <QFileInfo>
#include <QInputDialog>
#include <set>
#include "src/rendering/pdfdocument.h"

class PopplerDocument : public PdfDocument
{
    const Poppler::Document *doc {nullptr};

    /// Lookup table for page labels.
    std::set<int> overlay_slide_indices;

    /// Generate overlay_slide_indices.
    void populateOverlaySlidesSet();

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
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const override;
};

#endif // POPPLERDOCUMENT_H
