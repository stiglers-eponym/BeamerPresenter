// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <memory>
#include <utility>
#include <QString>
#include <QMap>
#include <QList>
#include <QtConfig>
#include <QCoreApplication>
#if (QT_VERSION_MAJOR == 6)
#include <poppler/qt6/poppler-qt6.h>
#elif (QT_VERSION_MAJOR == 5)
#include <poppler/qt5/poppler-qt5.h>
#endif
#include "src/config.h"
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"

class QPointF;
class QSizeF;
class QPixmap;
class PngPixmap;

/**
 * @brief Implement PdfDocument using Qt bindings of poppler
 * @implements PdfDocument
 * @see QtDocument
 * @see MuPdfDocument
 */
class PopplerDocument : public PdfDocument
{
    Q_DECLARE_TR_FUNCTIONS(PopplerDocument)

    /// Poppler document representing the PDF.
    std::unique_ptr<Poppler::Document> doc = NULL;

    /// Map page numbers to labels: Only the first page number with a
    /// new label is listed here.
    /// Exception: pages with an own TOC entry always have their label
    /// explicitly defined.
    QMap<int, QString> pageLabels;

    /// populate pageLabels. Must be called after loadOutline.
    void loadPageLabels();

    /// Load the PDF outline, fill PdfDocument::outline.
    void loadOutline();

public:
    /// Constructor: calls loadDocument().
    PopplerDocument(const QString &filename);

    /// Destructor: trivial
    ~PopplerDocument() noexcept override {}

    PdfEngine type() const noexcept override
    {return PopplerEngine;}

    /// Render page to QPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;

    /// Render page to PngPixmap. page is given as page index.
    /// resolution is given in pixels per point (dpi/72).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override final;

    /// Create a PopplerRenderer for this document.
    virtual AbstractRenderer *createRenderer(const PagePart part = FullPage) const override;

    /// Size of page in points (inch/72). Empty if page is invalid.
    const QSizeF pageSize(const int page) const override;

    /// Number of pages (0 if doc is null).
    int numberOfPages() const override
    {return doc ? doc->numPages() : 0;}

    /// Check whether a file has been loaded successfully.
    bool isValid() const override
    {return doc && !doc->isLocked();}

    /// Load the PDF labels and outline, fill PdfDocument::outline.
    void loadLabels() override;

    /// Search which page contains needle and return the
    /// outline of all occurrences on that slide.
    std::pair<int,QList<QRectF>> searchAll(const QString &needle, int start_page = 0, bool forward = true) const override;

    /// Page label of given page index. (Empty string if page is invalid.)
    const QString pageLabel(const int page) const override;

    /// Label of page with given index.
    int pageIndex(const QString &page) const override;

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    virtual QList<int> overlayIndices() const noexcept override
    {return pageLabels.size() == doc->numPages() ? QList<int>() : pageLabels.keys();}

    /// Link at given position (in point = inch/72).
    const PdfLink *linkAt(const int page, const QPointF &position) const override;

    /// List all video annotations on given page.
    virtual QList<MediaAnnotation> annotations(const int page) const override;

    /// Slide transition when reaching the given page.
    const SlideTransition transition(const int page) const override;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;

    /// Duration of given page in secons. Default value is -1 is interpreted as infinity.
    qreal duration(const int page) const noexcept override;
};

#endif // POPPLERDOCUMENT_H
