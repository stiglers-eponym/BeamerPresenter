// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef QTDOCUMENT_H
#define QTDOCUMENT_H

#include <QPdfDocumentRenderOptions>
#include <QCoreApplication>
#include <QPdfDocument>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    #define PAGESIZE_FUNCTION pagePointSize
#else
    #define PAGESIZE_FUNCTION pageSize
#endif

class QSizeF;
class QPixmap;
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
    Q_DECLARE_TR_FUNCTIONS(QtDocument)

    QPdfDocumentRenderOptions render_options;

    /// QtPdfDocument representing the PDF.
    QPdfDocument *doc = nullptr;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    /// Map page numbers to labels: Only the first page number with a
    /// new label is listed here.
    /// Exception: pages with an own TOC entry always have their label
    /// explicitly defined.
    QMap<int, QString> pageLabels;
#endif

public:
    /// Constructor: calls loadDocument().
    QtDocument(const QString &filename);

    /// Destructor: trivial
    ~QtDocument() noexcept override
    {delete doc;}

    PdfEngine type() const noexcept override
    {return QtPDFEngine;}

    /// Render page to QPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;

    /// Render page to PngPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override final;

    /// Create a QtRenderer for this document.
    virtual AbstractRenderer *createRenderer(const PagePart part = FullPage) const override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    /// populate pageLabels.
    void loadLabels() override;

    /// Label of page with given index.
    virtual const QString pageLabel(const int page) const override
    {return doc->pageLabel(page);}

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    virtual QList<int> overlayIndices() const noexcept override
    {return pageLabels.size() == doc->pageCount() ? QList<int>() : pageLabels.keys();}

    /// Label of page with given index.
    virtual int pageIndex(const QString &label) const override
    {
        if (pageLabels.isEmpty())
            return label.toInt() - 1;
        // this is slow, but possibly faster than the native implementation for some documents
        return pageLabels.key(label, -1);
    }
#endif // QT_VERSION >= 6.5

    /// Size of page in points (inch/72). Empty if page is invalid.
    const QSizeF pageSize(const int page) const override
    {return doc->PAGESIZE_FUNCTION(page);}

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
