// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include "src/config.h"
#include "src/enumerates.h"

class QPixmap;
class PngPixmap;

/// Abstract rendering class. Instances of implementing classes should be save
/// to use outside the main thread.
class AbstractRenderer
{
protected:
    /// Each page part of a document is associated to a single renderer.
    const PagePart page_part;

public:
    /// Type of the renderer.
    enum Renderer {
#ifdef USE_QTPDF
        QtPDF = 0,
#endif
#ifdef USE_POPPLER
        Poppler = 1,
#endif
#ifdef USE_MUPDF
        MuPDF = 2,
#endif
#ifdef USE_EXTERNAL_RENDERER
        ExternalRenderer = 3,
#endif
    };

    /// Constructor: only initializes page_part.
    AbstractRenderer(const PagePart part = FullPage) : page_part(part) {};

    /// Trivial virtual destructor.
    virtual ~AbstractRenderer() {};

    /// get page_part;
    PagePart pagePart() const
    {return page_part;}

    /// Render page to a QPixmap. Resolution is given in pixels per point
    /// (dpi/72).
    virtual const QPixmap renderPixmap(const int page, const qreal resolution) const = 0;

    /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    virtual const PngPixmap *renderPng(const int page, const qreal resolution) const = 0;

    /// Check if renderer is valid and can in principle render pages.
    virtual bool isValid() const = 0;
};

#endif // ABSTRACTRENDERER_H
