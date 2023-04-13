// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPixmap>
#include "src/gui/thumbnailthread.h"
#include "src/gui/thumbnailbutton.h"
#include "src/log.h"
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"
#include "src/rendering/abstractrenderer.h"
#ifdef USE_EXTERNAL_RENDERER
#include "src/rendering/externalrenderer.h"
#endif

ThumbnailThread::ThumbnailThread(const PdfDocument *document) :
    document(document)
{
    if (!document)
        return;

    // Create the renderer without any checks.
#ifdef USE_EXTERNAL_RENDERER
    if (preferences()->renderer == renderer::ExternalRenderer)
        renderer = new ExternalRenderer(preferences()->rendering_command, preferences()->rendering_arguments, document, preferences()->default_page_part);
    else
#endif
        renderer = document->createRenderer(preferences()->default_page_part);

    // Check if the renderer is valid
    if (renderer == nullptr || !renderer->isValid())
    {
        renderer = nullptr;
        qCritical() << tr("Creating renderer failed") << preferences()->renderer;
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
