// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef QTRENDERER_H
#define QTRENDERER_H

#include <QPixmap>
#include "src/config.h"
#include "src/rendering/abstractrenderer.h"

class PngPixmap;
class PdfDocument;
class QtDocument;

/**
 * @brief Implement AbstractRenderer using Qt PDF.
 * @implements AbstractRenderer
 * @see PopplerRenderer
 * @see MuPdfRenderer
 */
class QtRenderer : public AbstractRenderer
{
    /// Poppler PDF document.
    const QtDocument *doc;

public:
    /// Constructor: Only initializes doc and page_part.
    /// This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    QtRenderer(const PdfDocument *document, const PagePart part = FullPage);

    /// Trivial destructor.
    ~QtRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;

    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// Check whether doc is valid.
    bool isValid() const override;
};

#endif // QTRENDERER_H
