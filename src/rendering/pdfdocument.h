#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QDateTime>
#include <QObject>
#include "src/rendering/pngpixmap.h"

/// Abstract class for handling PDF documents.
/// This class uses Qt's signaling system to achieve thread safety and must
/// therefore be a QObject. That would not be necessary for Popper, but is
/// required for MuPDF.
class PdfDocument : public QObject
{
    Q_OBJECT

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
    explicit PdfDocument(const QString &filename, QObject *parent = nullptr) : QObject(parent), path(filename) {}
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
    virtual bool isValid() const = 0;
    const QString & getPath() const {return path;}

public slots:
    void getPageSize(QSizeF *size, const int page) {*size = pageSize(page);}
};

#endif // PDFDOCUMENT_H
