// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef QTRENDERER_H
#define QTRENDERER_H

#include <QPixmap>
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/qtdocument.h"

/**
 * @brief Implement AbstractRenderer using Qt PDF.
 * @implements AbstractRenderer
 * @see PopplerRenderer
 */
class QtRenderer : public AbstractRenderer
{
    /// Poppler PDF document.
    const QtDocument *doc;

public:
    /// Constructor: Only initializes doc and page_part.
    /// This does not perform any checks on the given document.
    /// Also rendering hints are not set here and have to be set before.
    QtRenderer(const QtDocument *document, const PagePart part = FullPage) :
        AbstractRenderer(part), doc(document) {};

    /// Trivial destructor.
    ~QtRenderer() override {}

    /// Render page to a QPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override
    {return doc ? doc->getPixmap(page, resolution, page_part) : QPixmap();}

    /// Render page to PNG image in a QByteArray.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override
    {return doc ? doc->getPng(page, resolution, page_part) : NULL;}

    /// Check whether doc is valid.
    bool isValid() const override
    {return doc && doc->isValid();}
};

#endif // QTRENDERER_H
