#ifndef MUPDFDOCUMENT_H
#define MUPDFDOCUMENT_H

#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <QFileInfo>
#include "src/rendering/pdfdocument.h"

class MuPdfDocument : public PdfDocument
{
    /// context should be cloned for each separate thread.
    fz_context *context = nullptr;
    /// document is global, don't clone if for threads.
    fz_document *doc = nullptr;
    int number_of_pages;
public:
    MuPdfDocument(const QString &filename);
    ~MuPdfDocument() override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const QPixmap getPixmap(const int page, const qreal resolution) const override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const PngPixmap* getPng(const int page, const qreal resolution) const override;
    bool loadDocument() override;
    /// Size of page in points (72*inch).
    const QSizeF pageSize(const int page) const override;
    int numberOfPages() const override {return number_of_pages;}
    bool isValid() const override;
    const QString label(const int page) const override;
    fz_context* getContext() const {return context;}
    fz_document* getDocument() const {return doc;}
};

#endif // MUPDFDOCUMENT_H
