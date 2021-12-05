#include "src/rendering/pixcachethread.h"
#ifdef USE_POPPLER
#include "src/rendering/popplerrenderer.h"
#include "src/rendering/popplerdocument.h"
#endif
#ifdef USE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#include "src/rendering/mupdfdocument.h"
#endif
#include "src/rendering/externalrenderer.h"
#include "src/rendering/pngpixmap.h"
#include "src/preferences.h"

PixCacheThread::PixCacheThread(const PdfDocument * const doc, const PagePart page_part, QObject *parent) : QThread(parent)
{
    initializeRenderer(doc, page_part);
}

PixCacheThread::~PixCacheThread()
{
    delete renderer;
}

void PixCacheThread::setNextPage(const int page_number, const qreal res)
{
    page = page_number;
    resolution = res;
}

void PixCacheThread::run()
{
    // Check if a renderer is available.
    if (renderer == NULL || resolution <= 0. || page < 0)
        return;

    // Render the image. This is takes some time.
    debug_msg(DebugCache, "Rendering in cache thread:" << page << resolution << this);
    const PngPixmap *image = renderer->renderPng(page, resolution);

    // Send the image to pixcache master.
    if (image)
        emit sendData(image);
}

bool PixCacheThread::initializeRenderer(const PdfDocument * const doc, const PagePart page_part)
{
    // Create the renderer without any checks.
    switch (preferences()->renderer)
    {
#ifdef USE_POPPLER
    case AbstractRenderer::Poppler:
        renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(doc), page_part);
        break;
#endif
#ifdef USE_MUPDF
    case AbstractRenderer::MuPDF:
        renderer = new MuPdfRenderer(static_cast<const MuPdfDocument*>(doc), page_part);
        break;
#endif
    case AbstractRenderer::ExternalRenderer:
        renderer = new ExternalRenderer(preferences()->rendering_command, preferences()->rendering_arguments, doc, page_part);
        break;
    }

    // Check if the renderer was created successfully.
    if (renderer->isValid())
        return true;

    // Creating renderer failed. Clean up and return false.
    qCritical() << "Creating renderer failed" << preferences()->renderer;
    delete renderer;
    renderer = NULL;
    return false;
}
