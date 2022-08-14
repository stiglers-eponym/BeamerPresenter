// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/thumbnailthread.h"
#include "src/gui/thumbnailbutton.h"
#include "src/preferences.h"
#ifdef USE_QTPDF
#include "src/rendering/qtrenderer.h"
#endif
#ifdef USE_POPPLER
#include "src/rendering/popplerrenderer.h"
#endif
#ifdef USE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#endif
#ifdef USE_EXTERNAL_RENDERER
#include "src/rendering/externalrenderer.h"
#endif

ThumbnailThread::ThumbnailThread(const PdfDocument *document) :
    document(document)
{
    if (!document)
        return;

    // Create the renderer without any checks.
    switch (preferences()->renderer)
    {
#ifdef USE_QTPDF
    case AbstractRenderer::QtPDF:
        renderer = new QtRenderer(static_cast<const QtDocument*>(document), preferences()->default_page_part);
        break;
#endif
#ifdef USE_POPPLER
    case AbstractRenderer::Poppler:
        renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(document), preferences()->default_page_part);
        break;
#endif
#ifdef USE_MUPDF
    case AbstractRenderer::MuPDF:
        renderer = new MuPdfRenderer(static_cast<const MuPdfDocument*>(document), preferences()->default_page_part);
        break;
#endif
#ifdef USE_EXTERNAL_RENDERER
    case AbstractRenderer::ExternalRenderer:
        renderer = new ExternalRenderer(preferences()->rendering_command, preferences()->rendering_arguments, document, preferences()->default_page_part);
        break;
#endif
    }

    // Check if the renderer is valid
    if (renderer == NULL || !renderer->isValid())
    {
        renderer = NULL;
        qCritical() << "Creating renderer failed" << preferences()->renderer;
        return;
    }
}

void ThumbnailThread::renderImages()
{
    if (!renderer || !document)
        return;
    queue_entry entry;
    while (!queue.isEmpty())
    {
        entry = queue.takeFirst();
        if (entry.button && entry.resolution)
            emit sendThumbnail(entry.button, renderer->renderPixmap(entry.page, entry.resolution));
            //entry.button->setPixmap(renderer->renderPixmap(entry.page, entry.resolution));
    }
}
