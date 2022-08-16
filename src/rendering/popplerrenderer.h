// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef POPPLERRENDERER_H
#define POPPLERRENDERER_H

#include <QPixmap>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/rendering/abstractrenderer.h"

class PngPixmap;
class PdfDocument;
class PopplerDocument;

/**
 * @brief Implement AbstractRenderer using Poppler.
 * @implements AbstractRenderer
 * @see QtRenderer
 * @see MuPdfRenderer
 */
class PopplerRenderer : public AbstractRenderer
{
    /// Poppler PDF document.
    const PopplerDocument *doc;

public:
    /// Constructor: Only initializes doc and page_part.
    /// This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    PopplerRenderer(const PdfDocument *document, const PagePart part = FullPage);

    /// Trivial destructor.
    ~PopplerRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;

    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// Check whether doc is valid.
    bool isValid() const override;
};

#endif // POPPLERRENDERER_H
