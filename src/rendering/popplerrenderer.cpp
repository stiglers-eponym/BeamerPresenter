// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/popplerrenderer.h"
#include "src/rendering/popplerdocument.h"

PopplerRenderer::PopplerRenderer(const PdfDocument *document, const PagePart part) :
    AbstractRenderer(part),
    doc(document && (document->type() == PopplerEngine) ? static_cast<const PopplerDocument*>(document) : nullptr)
{};


const QPixmap PopplerRenderer::renderPixmap(const int page, const qreal resolution) const
{
    return doc ? doc->getPixmap(page, resolution, page_part) : QPixmap();
}

const PngPixmap * PopplerRenderer::renderPng(const int page, const qreal resolution) const
{
    return doc ? doc->getPng(page, resolution, page_part) : nullptr;
}

bool PopplerRenderer::isValid() const
{
    return doc && doc->isValid();
}
