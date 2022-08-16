// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/thumbnailthread.h"
#include "src/gui/thumbnailbutton.h"
#include "src/log.h"
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
    case renderer::QtPDF:
        renderer = new QtRenderer(document, preferences()->default_page_part);
        break;
#endif
#ifdef USE_POPPLER
    case renderer::Poppler:
        renderer = new PopplerRenderer(document, preferences()->default_page_part);
        break;
#endif
#ifdef USE_MUPDF
    case renderer::MuPDF:
        renderer = new MuPdfRenderer(document, preferences()->default_page_part);
        break;
#endif
#ifdef USE_EXTERNAL_RENDERER
    case renderer::ExternalRenderer:
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

ThumbnailThread::~ThumbnailThread()
{
    delete renderer;
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
