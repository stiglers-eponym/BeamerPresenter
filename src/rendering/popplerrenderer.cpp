#include "src/rendering/popplerrenderer.h"

const QPixmap PopplerRenderer::renderPixmap(const int page, const qreal resolution) const
{
    return doc->getPixmap(page, resolution, page_part);
}

const PngPixmap * PopplerRenderer::renderPng(const int page, const qreal resolution) const
{
    return doc->getPng(page, resolution, page_part);
}
