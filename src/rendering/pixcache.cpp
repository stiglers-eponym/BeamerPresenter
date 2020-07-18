#include "src/rendering/pixcache.h"
#ifdef INCLUDE_POPPLER
#include "src/rendering/popplerrenderer.h"
#endif
#ifdef INCLUDE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#endif
#include "src/rendering/externalrenderer.h"

PixCache::PixCache(const PdfDocument *doc, const int thread_number, QObject *parent) :
    QObject(parent),
    pdfDoc(doc)
{
    threads = QVector<PixCacheThread*>(thread_number);

    // Create the renderer without any checks.
    switch (preferences().renderer)
    {
#ifdef INCLUDE_POPPLER
    case AbstractRenderer::Poppler:
        renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(pdfDoc));
        break;
#endif
#ifdef INCLUDE_MUPDF
    case AbstractRenderer::MuPDF:
        renderer = new MuPdfRenderer(static_cast<const MuPdfDocument*>(pdfDoc));
        break;
#endif
    case AbstractRenderer::ExternalRenderer:
        renderer = new ExternalRenderer(preferences().rendering_command, preferences().rendering_arguments, pdfDoc);
        break;
    }

    // Check if the renderer is valid
    if (renderer == nullptr || !renderer->isValid())
    {
        qCritical() << "Creating renderer failed" << preferences().renderer;
    }
}

void PixCache::init()
{
    qDebug() << "init";
    // Create threads.
    for (int i=0; i<threads.length(); i++)
    {
        threads[i] = new PixCacheThread(pdfDoc, this);
        connect(threads[i], &PixCacheThread::sendData, this, &PixCache::receiveData);
    }

    renderCacheTimer = new QTimer(this);
    renderCacheTimer->setSingleShot(true);
    renderCacheTimer->setInterval(0);
    connect(renderCacheTimer, &QTimer::timeout, this, &PixCache::startRendering);
    qDebug() << "done";
}

PixCache::~PixCache()
{
    delete renderCacheTimer;
    delete renderer;
    // TODO: correctly clean up threads!
    for (auto &thread : threads)
        delete thread;
    clear();
}

void PixCache::setMaxMemory(const float memory)
{
    maxMemory = memory;
    if (memory < usedMemory && memory >= 0)
        limitCacheSize();
}

void PixCache::setMaxNumber(const int number)
{
    maxNumber = number;
    if (number < usedMemory && number >= 0)
        limitCacheSize();
}

void PixCache::clear()
{
    qDeleteAll(cache);
    cache.clear();
    usedMemory = 0;
    region = {preferences().page, preferences().page};
}

const QPixmap PixCache::pixmap(const int page) const
{
    qreal const resolution = getResolution(page);
    // Try to return a page from cache.
    {
        const auto it = cache.constFind(page);
        if (it != cache.cend() && *it != nullptr && abs((*it)->getResolution() - resolution) < MAX_RESOLUTION_DEVIATION)
            return (*it)->pixmap();
    }

    // Check if page number is valid.
    if (page < 0 || page >= pdfDoc->numberOfPages())
        return QPixmap();

    // Check if the renderer is valid
    if (renderer == nullptr || !renderer->isValid())
    {
        qCritical() << "Invalid renderer";
        return QPixmap();
    }

    const QPixmap pix = renderer->renderPixmap(page, resolution);

    if (pix.isNull())
        qCritical() << "Rendering page failed" << page << resolution;

    // The page is not written to cache!
    return pix;
}

const QPixmap PixCache::pixmap(const int page)
{
    qreal const resolution = getResolution(page);
    // Try to return a page from cache.
    {
        const auto it = cache.constFind(page);
        if (it != cache.cend() && *it != nullptr && abs((*it)->getResolution() - resolution) < MAX_RESOLUTION_DEVIATION)
            return (*it)->pixmap();
    }
    // Check if page number is valid.
    if (page < 0 || page >= pdfDoc->numberOfPages())
        return QPixmap();

    // Check if the renderer is valid
    if (renderer == nullptr || !renderer->isValid())
    {
        qCritical() << "Invalid renderer";
        return QPixmap();
    }

    const QPixmap pix = renderer->renderPixmap(page, resolution);

    if (pix.isNull())
        qCritical() << "Rendering page failed" << page << resolution;

    // Write pixmap to cache.
    const PngPixmap *png = new PngPixmap(pix, page, resolution);
    if (png == nullptr)
        qWarning() << "Converting pixmap to PNG failed";
    else
        cache[page] = png;

    return pix;
}

void PixCache::requestRenderPage(const int n)
{
    if (!priority.contains(n) && !cache.contains(n))
        priority.append(n);

    // Start rendering next page.
    renderCacheTimer->start();
}

void PixCache::updatePageNumber(const int page_number)
{
    currentPage = page_number;

    // Update boundaries of the simply connected region.
    if (!cache.contains(currentPage))
    {
        // If current page is not yet in cache: make sure it is first in priority queue.
        if (priority.first() != currentPage)
        {
            priority.removeOne(currentPage);
            priority.push_front(currentPage);
        }
        region.first = currentPage;
        region.second = currentPage;
        return;
    }

    // Make sure that current page is inside the region.
    if (region.first > currentPage || region.second < currentPage) {
        region.first = currentPage - 1;
        region.second = currentPage + 1;
    }

    // Extend the region as far as possible by searching for gaps.
    // Use that keys in QMap are sorted and can be accessed by iterators.
    QMap<int, const PngPixmap*>::const_iterator
            left = cache.find(region.first),
            right = cache.find(region.second);
    if (left != cache.cend())
    {
        const auto limit = cache.cbegin() - 1;
        while (left != limit && left.key() == region.first)
        {
            --left;
            --region.first;
        }
    }
    while (right != cache.cend() && right.key() == region.second)
    {
        ++right;
        ++region.second;
    }

    // Start rendering next page.
    renderCacheTimer->start();
}

int PixCache::limitCacheSize()
{
    // Check restrictions on memory usage and number of slides.
    if (maxMemory < 0 && maxNumber < 0)
    {
        // Check if all pages are already in memory.
        if (cache.size() == pdfDoc->numberOfPages())
            return 0;
        return INT_MAX;
    }
    if (maxNumber == 0 || maxMemory == 0)
    {
        clear();
        return 0;
    }

    // Check if region is valid.
    if (region.first > region.second)
    {
        region.first = currentPage;
        region.second = currentPage;
    }

    // Number of really cached slides:
    // subtract number of currently active threads, for which cache contains a nullptr.
    int cached_slides = cache.size();
    for (QVector<PixCacheThread*>::const_iterator it = threads.cbegin(); it != threads.cend(); ++it)
    {
        if ((*it)->isRunning())
            --cached_slides;
    }
    if (cached_slides <= 0)
        return INT_MAX;

    // Calculate how many slides still fit in available memory
    int allowed_slides = INT_MAX;
    if (maxMemory > 0)
    {
        // Get currently used memory.
        // Get number of slides which still fit in available memory.
        if (usedMemory > 0 && cached_slides > 0)
            allowed_slides = (maxMemory - usedMemory) * cached_slides / usedMemory;
        else
            allowed_slides = threads.length();
    }
    if (maxNumber > 0 && allowed_slides + cached_slides > maxNumber)
        allowed_slides = maxNumber - cached_slides;

    // If threads.length() pages can be rendered without problems: return
    if (allowed_slides >= threads.length())
        return allowed_slides;


    // Deleting starts from first or last page in cache.
    // The aim is to shrink the cache to a simply connected region
    // around the current page.
    int first = cache.firstKey();
    int last  = cache.lastKey();
    /// Cached page which should be removed.
    const PngPixmap *remove;

    // Delete pages while allowed_slides is negative or too small to
    // allow updates.
    do {
        // If the set of cached pages is simply connected, includes the
        // current page, and lies mostly ahead of the current page,
        // then clean stop cleaning up.
        if (last > currentPage && last - first <= cache.size() && 2*last + 3*first > 5*currentPage)
            break;

        // If more than 3/4 of the cached slides lie ahead of current page, clean up last.
        if (last + 3*first > 4*currentPage)
        {
            auto it = cache.end() - 1;
            remove = it.value();
            last = (cache.erase(it) - 1).key();
        }
        else
        {
            auto it = cache.begin();
            remove = it.value();
            first = cache.erase(it).key();
        }
        // Check if remove is nullptr (which means that a thread is just rendering it).
        // TODO: make sure this case is correctly handled when the thread finishes.
        if (remove == nullptr)
            continue;
        // Delete removed cache page and update memory size.
        usedMemory -= remove->size();
        delete remove;
        --cached_slides;

        // Update allowed_slides
        if (usedMemory > 0 && cached_slides > 0)
            allowed_slides = (maxMemory - usedMemory) * cached_slides / usedMemory;
        if (allowed_slides + cached_slides > maxNumber)
            allowed_slides = maxNumber - cached_slides;

    } while (allowed_slides < threads.length() && cached_slides > 0);

    // Update boundaries of simply connected region
    if (first > region.first + 1)
        region.first = first - 1;
    if (last + 1 < region.second)
        region.second = last + 1;

    return allowed_slides;
}

int PixCache::renderNext()
{
    // Check if priority contains pages which are not yet rendered.
    int page;
    while (!priority.isEmpty())
    {
        page = priority.takeFirst();
        if (!cache.contains(page))
            return page;
    }

    // Check if region is valid.
    if (region.first > region.second)
    {
        region.first = currentPage;
        region.second = currentPage;
    }

    // Select region.first or region.second for rendering.
    while (true)
    {
        if (region.second + 3*region.first > 4*currentPage && region.first >= 0)
        {
            if (!cache.contains(region.first))
                return region.first--;
            --region.first;
        }
        else
        {
            if (!cache.contains(region.second))
                return region.second++;
            ++region.second;
        }
    }
}

void PixCache::startRendering()
{
    qDebug() << "Start rendering to cache";
    // Clean up cache and check if there is enough space for more cached pages.
    int allowed_pages = limitCacheSize();
    if (allowed_pages <= 0)
        return;
    for (QVector<PixCacheThread*>::const_iterator thread = threads.cbegin(); thread != threads.cend(); ++thread)
    {
        if (allowed_pages > 0 && !(*thread)->isRunning())
        {
            const int page = renderNext();
            if (page < 0 || page >= pdfDoc->numberOfPages())
                return;
            (*thread)->setNextPage(page, getResolution(page));
            (*thread)->start(QThread::LowPriority);
            --allowed_pages;
        }
    }
}

void PixCache::receiveData(const PngPixmap *data)
{
    // If a renderer failed, it should already have sent an error message.
    if (data == nullptr || data->isNull())
        return;

    // Check if the received image is still compatible with the current resolution.
    if (abs(getResolution(data->getPage()) - data->getResolution()) > MAX_RESOLUTION_DEVIATION)
    {
        if (cache.value(data->getPage()) == nullptr)
            cache.remove(data->getPage());
        delete data;
    }
    else
    {
        cache[data->getPage()] = data;
    }

    // Start rendering next page.
    renderCacheTimer->start();
}

qreal PixCache::getResolution(const int page) const
{
    // Get page size in points
    QSizeF pageSize = pdfDoc->pageSize(page);
    if (pageSize.isEmpty())
        return -1.;
    if (preferences().page_part != FullPage)
        pageSize.rwidth() /= 2;
    if (pageSize.width() * frame.height() > pageSize.height() * frame.width())
    {
        // page is too wide, determine resolution by x direction
        return frame.width() / pageSize.width();
    }
    // page is too high, determine resolution by y direction
    return frame.height() / pageSize.height();
}

void PixCache::updateFrame(const QSizeF &size)
{
    qDebug() << "update frame";
    if (frame != size)
    {
        frame = size;
        cache.clear();
    }
}

void PixCache::requestPage(const int page, const qreal resolution)
{
    qDebug() << "requested page" << page << resolution;
    // Try to return a page from cache.
    {
        const auto it = cache.constFind(page);
        if (it != cache.cend() && *it != nullptr && abs((*it)->getResolution() - resolution) < MAX_RESOLUTION_DEVIATION)
        {
            emit pageReady((*it)->pixmap(), page);
            return;
        }
    }
    // Check if page number is valid.
    if (page < 0 || page >= pdfDoc->numberOfPages())
        return;

    // Render new page.
    // Check if the renderer is valid
    if (renderer == nullptr || !renderer->isValid())
    {
        qCritical() << "Invalid renderer";
        return;
    }

    const QPixmap pix = renderer->renderPixmap(page, resolution);

    if (pix.isNull())
    {
        qCritical() << "Rendering page failed" << page << resolution;
        return;
    }

    emit pageReady(pix, page);

    // Write pixmap to cache.
    const PngPixmap *png = new PngPixmap(pix, page, resolution);
    if (png == nullptr)
        qWarning() << "Converting pixmap to PNG failed";
    else
        cache[page] = png;

    // Start rendering next page.
    renderCacheTimer->start();
}
