#include "src/rendering/pixcachethread.h"
#ifdef INCLUDE_POPPLER
#include "src/rendering/popplerrenderer.h"
#endif
#ifdef INCLUDE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#endif
#include "src/rendering/externalrenderer.h"
#include "src/preferences.h"

PixCacheThread::PixCacheThread(const PdfDocument * const doc, QObject *parent) : QThread(parent)
{
    initializeRenderer(doc);
}

void PixCacheThread::setNextPage(const int page_number, const qreal res)
{
    page = page_number;
    resolution = res;
}

void PixCacheThread::run()
{
    qDebug() << "Run thread" << renderer;
    // Check if a renderer is available.
    if (renderer == nullptr)
        return;

    // Copy the variable page and resolution.
    // They might be overwritten by another thread while rendering.

    // Render the image. This is takes some time.
    // TODO: exception handling.
    const PngPixmap *image = renderer->renderPng(page, resolution);

    // Send the image to "master".
    emit sendData(image);
}

bool PixCacheThread::initializeRenderer(const PdfDocument * const doc)
{
    // Create the renderer without any checks.
    switch (preferences().renderer)
    {
#ifdef INCLUDE_POPPLER
    case AbstractRenderer::Poppler:
        renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(doc));
        break;
#endif
#ifdef INCLUDE_MUPDF
    case AbstractRenderer::MuPDF:
        renderer = new MuPdfRenderer();
        connect(
                    static_cast<MuPdfRenderer*>(renderer),
                    &MuPdfRenderer::prepareRendering,
                    static_cast<const MuPdfDocument*>(doc),
                    &MuPdfDocument::prepareRendering,
                    Qt::BlockingQueuedConnection
                    );
        break;
#endif
    case AbstractRenderer::ExternalRenderer:
        renderer = new ExternalRenderer(preferences().rendering_command, preferences().rendering_arguments, doc);
        break;
    }

    // Check if the renderer was created successfully.
    if (renderer->isValid())
        return true;

    // Creating renderer failed. Clean up and return false.
    qCritical() << "Creating renderer failed" << preferences().renderer;
    delete renderer;
    renderer = nullptr;
    return false;
}
