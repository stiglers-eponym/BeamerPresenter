// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef QTDOCUMENT_H
#define QTDOCUMENT_H

#include <QtGlobal>
#include <QPdfDocumentRenderOptions>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"

class QSizeF;
class QPixmap;
class PngPixmap;
class QPdfDocument;

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
    QPdfDocument *doc = nullptr;

public:
    /// Constructor: calls loadDocument().
    QtDocument(const QString &filename);

    /// Destructor: trivial
    ~QtDocument() noexcept override;

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

    /// Size of page in points (inch/72). Empty if page is invalid.
    const QSizeF pageSize(const int page) const override;

    /// Number of pages (0 if doc is null).
    int numberOfPages() const override;

    /// Check whether a file has been loaded successfully.
    bool isValid() const override;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;
};

#endif // QTDOCUMENT_H
