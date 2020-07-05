#include "pixcache.h"

PixCache::PixCache(const int thread_number, QObject *parent) :
    QObject(parent),
    threads(QVector<PixCacheThread*>(thread_number))
{
    // Create threads.
    for (int i=0; i<thread_number; i++) {
        threads[i] = new PixCacheThread(this);
        connect(threads[i], &PixCacheThread::sendData, this, &PixCache::receiveData);
    }
}

PixCache::~PixCache()
{
    // TODO: correctly clean up threads!
    for (auto thread : threads)
        delete thread;
    clear();
}

void PixCache::clear()
{
    qDeleteAll(cache);
    cache.clear();
    usedMemory = 0;
    region = {INT_MAX, -1};
}

QPixmap* PixCache::pixmap(const int n) const
{
    if (cache.contains(n))
        return cache[n]->pixmap();
    // TODO: render!
}

void PixCache::setResolution(const qreal &new_resolution)
{
    if (abs(resolution - new_resolution) > 1e-9)
        clear();
    resolution = new_resolution;
}

void PixCache::requestRenderPage(const int n)
{
    if (!priority.contains(n) && !cache.contains(n))
        priority.append(n);

    // Start rendering next page.
    startRendering();
}

void PixCache::updatePageNumber(const int page_number)
{
    currentPage = page_number;

    // Update boundaries of the simply connected region.
    if (!cache.contains(currentPage)) {
        // If current page is not yet in cache: make sure it is first in priority queue.
        if (priority.first() != currentPage) {
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
    QMap<int, const PngPixmap*>::const_iterator left = cache.find(region.first), right = cache.find(region.second);
    if (left != cache.cend()) {
        const auto limit = cache.cbegin() - 1;
        while (left != limit && left.key() == region.first) {
            --left;
            --region.first;
        }
    }
    while (right != cache.cend() && right.key() == region.second) {
        ++right;
        ++region.second;
    }

    // Start rendering next page.
    startRendering();
}

int PixCache::limitCacheSize()
{
    // Check restrictions on memory usage and number of slides
    if (maxMemory < 0 && maxNumber < 0)
        return -1;
    if (maxNumber == 0 || maxMemory == 0) {
        clear();
        return 0;
    }

    // Number of really cached slides:
    // subtract number of currently active threads, for which cache contains a nullptr.
    int cached_slides = cache.size();
    for (QVector<PixCacheThread*>::const_iterator it = threads.cbegin(); it != threads.cend(); ++it) {
        if ((*it)->isRunning())
            --cached_slides;
    }
    if (cached_slides <= 0)
        return -1;

    // Calculate how many slides still fit in available memory
    int allowed_slides = INT_MAX;
    if (maxMemory > 0) {
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
        if (last + 3*first > 4*currentPage) {
            auto it = cache.end() - 1;
            remove = it.value();
            last = (cache.erase(it) - 1).key();
        }
        else {
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
    while (!priority.isEmpty()) {
        page = priority.takeFirst();
        if (!cache.contains(page))
            return page;
    }

    // Select region.first or region.second for rendering.
    if (region.second + 3*region.first > 4*currentPage ) {
        return region.first--;
    }
    else {
        return region.second++;
    }
}

void PixCache::startRendering()
{
    // Clean up cache and check if there is enough space for more cached pages.
    int allowed_pages = limitCacheSize();
    for (QVector<PixCacheThread*>::const_iterator thread = threads.cbegin(); thread != threads.cend(); ++thread) {
        if (allowed_pages > 0 && !(*thread)->isRunning()) {
            (*thread)->setPageNumber(renderNext());
            (*thread)->start();
            --allowed_pages;
        }
    }
}

void PixCache::receiveData(const PngPixmap * const data)
{
    // Check if the received image is still compatible with the current resolution.
    if (abs(resolution - data->getResolution()) > 1e-9) {
        if (cache.value(data->getPage()) == nullptr)
            cache.remove(data->getPage());
        delete data;
    }
    else {
        cache.insert(data->getPage(), data);
    }

    // Start rendering next page.
    startRendering();
}
