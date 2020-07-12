#include "src/rendering/pixcachethread.h"
#include "src/pdfmaster.h"
#include "src/rendering/popplerrenderer.h"
#include "src/rendering/mupdfrenderer.h"
#include "src/rendering/externalrenderer.h"
#include "src/preferences.h"

PixCacheThread::PixCacheThread(const PdfMaster * const master, QObject *parent) : QThread(parent)
{
    initializeRenderer(master);
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
    const int page_copy = page;
    const qreal resolution_copy = resolution;

    // Render the image. This is takes some time.
    // TODO: exception handling.
    const QByteArray *data = renderer->renderPng(page_copy, resolution_copy);

    // Convert to PngPixmap. Here it is important that we use the copied variables.
    const PngPixmap *image = new PngPixmap(data, page_copy, resolution_copy);
    // Send the image to "master".
    emit sendData(image);
}

bool PixCacheThread::initializeRenderer(const PdfMaster * const master)
{
    // Create the renderer without any checks.
    switch (preferences().renderer)
    {
    case AbstractRenderer::Poppler:
        renderer = new PopplerRenderer(master->getDocument());
        break;
#ifdef INCLUDE_MUPDF
    case AbstractRenderer::MuPDF:
        renderer = new MuPdfRenderer(master->getFilename());
        break;
#endif
    case AbstractRenderer::ExternalRenderer:
        renderer = new ExternalRenderer(preferences().rendering_command, preferences().rendering_arguments, master);
        break;
    }

    // Check if the renderer was created successfully.
    if (renderer->isValid())
        return true;

    // Creating renderer failed. Clean up and return false.
    qCritical() << "Creating renderer failed";
    delete renderer;
    renderer = nullptr;
    return false;
}
