#include "thumbnailthread.h"

ThumbnailThread::ThumbnailThread(const PdfDocument *document) :
    document(document)
{
    if (!document)
        return;

    // Create the renderer without any checks.
    if (!renderer)
    {
        switch (preferences().renderer)
        {
#ifdef INCLUDE_POPPLER
        case AbstractRenderer::Poppler:
            renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(document), preferences().default_page_part);
            break;
#endif
#ifdef INCLUDE_MUPDF
        case AbstractRenderer::MuPDF:
            renderer = new MuPdfRenderer(static_cast<const MuPdfDocument*>(document), preferences().default_page_part);
            break;
#endif
        case AbstractRenderer::ExternalRenderer:
            renderer = new ExternalRenderer(preferences().rendering_command, preferences().rendering_arguments, document, preferences().default_page_part);
            break;
        }

        // Check if the renderer is valid
        if (renderer == NULL || !renderer->isValid())
        {
            renderer = NULL;
            qCritical() << "Creating renderer failed" << preferences().renderer;
            return;
        }
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
