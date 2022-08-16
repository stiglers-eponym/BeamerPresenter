// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/qtrenderer.h"
#include "src/rendering/qtdocument.h"

QtRenderer::QtRenderer(const PdfDocument *document, const PagePart part) :
    AbstractRenderer(part),
    doc(document && (document->type() == QtPDFEngine) ? static_cast<const QtDocument*>(document) : nullptr)
{};

const QPixmap QtRenderer::renderPixmap(const int page, const qreal resolution) const
{
    return doc ? doc->getPixmap(page, resolution, page_part) : QPixmap();
}

const PngPixmap * QtRenderer::renderPng(const int page, const qreal resolution) const
{
    return doc ? doc->getPng(page, resolution, page_part) : nullptr;
}

bool QtRenderer::isValid() const
{
    return doc && doc->isValid();
}
