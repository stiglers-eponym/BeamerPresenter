#ifndef QTDOCUMENT_H
#define QTDOCUMENT_H

#include <QObject>
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
#include <QtPdf>
#else
#include <QtPdfDocument>
#endif
#include "src/enumerates.h"
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"

class PngPixmap;

/**
 * @brief Implement PdfDocument using Qt PDF
 *
 * Minimal implementation of PdfDocument using only Qt libraries.
 * This only implements a minimal set of features.
 *
 * @implements PdfDocument
 * @see PopplerDocument
 */
class QtDocument : public PdfDocument
{
    QPdfDocumentRenderOptions render_options;

    /// QtPdfDocument representing the PDF.
    QPdfDocument *doc = NULL;

public:
    /// Constructor: calls loadDocument().
    QtDocument(const QString &filename);

    /// Destructor: trivial
    ~QtDocument() noexcept override
    {delete doc;}

    /// Render page to QPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;

    /// Render page to PngPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override final;

    /// Size of page in points (inch/72). Empty if page is invalid.
    const QSizeF pageSize(const int page) const override
    {return doc->pageSize(page);}

    /// Number of pages (0 if doc is null).
    int numberOfPages() const override
    {return doc->pageCount();}

    /// Check whether a file has been loaded successfully.
    bool isValid() const override
    {return doc->status() == QPdfDocument::Status::Ready;}

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;
};

#endif // QTDOCUMENT_H
