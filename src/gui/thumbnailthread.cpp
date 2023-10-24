// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPixmap>
#include "src/gui/thumbnailthread.h"
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
        renderer = new ExternalRenderer(
                preferences()->rendering_command,
                preferences()->rendering_arguments,
                document,
                preferences()->default_page_part);
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

void ThumbnailThread::timerEvent(QTimerEvent* event)
{
    if (!renderer || queue.isEmpty())
        killTimer(event->timerId());
    else
    {
        queue_entry entry = queue.takeFirst();
        emit sendThumbnail(
                entry.button_index,
                renderer->renderPixmap(entry.page, entry.resolution));
    }
}
