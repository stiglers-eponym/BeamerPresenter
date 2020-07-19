#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QDateTime>
#include "src/rendering/pngpixmap.h"

struct PdfLink {
    /// Positive values of type are interpreted as page numbers.
    /// Negative values are interpreted as LinkType.
    int type;
    QString target;
};

/// Abstract class for handling PDF documents.
class PdfDocument
{

protected:
    QDateTime lastModified;
    QString path;

public:
    enum PdfBackend {
#ifdef INCLUDE_POPPLER
        PopplerBackend = 0,
#endif
#ifdef INCLUDE_MUPDF
        MuPdfBackend = 1,
#endif
    };
    explicit PdfDocument(const QString &filename) : path(filename) {}
    virtual ~PdfDocument() {}
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    virtual const QPixmap getPixmap(const int page, const qreal resolution) const = 0;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    virtual const PngPixmap* getPng(const int page, const qreal resolution) const = 0;
    virtual bool loadDocument() = 0;
    /// Size of page in points (72*inch).
    virtual const QSizeF pageSize(const int page) const = 0;
    virtual int numberOfPages() const = 0;
    virtual const QString label(const int page) const = 0;
    virtual int overlaysShifted(const int start, const int shift_overlay) const = 0;
    virtual bool isValid() const = 0;
    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const = 0;
    const QString & getPath() const {return path;}
};

#endif // PDFDOCUMENT_H
