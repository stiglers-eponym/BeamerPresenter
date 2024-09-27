// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/pixcache.h"

#include <QPixmap>
#include <QThread>
#include <QTimerEvent>
#include <utility>

#include "src/config.h"
#include "src/log.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"
#ifdef USE_EXTERNAL_RENDERER
#include "src/rendering/externalrenderer.h"
#endif
#include "src/preferences.h"
#include "src/rendering/pixcachethread.h"
#include "src/rendering/pngpixmap.h"

PixCache::PixCache(const std::shared_ptr<PdfDocument> &doc,
                   const int thread_number, const PagePart page_part,
                   const CacheMode mode, QObject *parent) noexcept
    : QObject(parent), priority({page_part}), pdfDoc(doc), cacheMode(mode)
{
  debug_verbose(DebugFunctionCalls, "CREATING PixCache" << this);
  threads =
      QVector<PixCacheThread *>(doc->flexiblePageSizes() ? 0 : thread_number);
  threads.fill(nullptr);
}

void PixCache::init()
{
  debug_verbose(DebugFunctionCalls, this);
  if (QThread::currentThread() != this->thread()) {
    qCritical() << "Called PixCache::init from wrong thread!";
    return;
  }
  mutex.lock();
  const PagePart page_part =
      static_cast<PagePart>(priority.isEmpty() ? 0 : priority.first());
  priority.clear();
  // Create the renderer without any checks.
#ifdef USE_EXTERNAL_RENDERER
  if (preferences()->renderer == renderer::ExternalRenderer)
    renderer = new ExternalRenderer(preferences()->rendering_command,
                                    preferences()->rendering_arguments, pdfDoc,
                                    page_part);
  else
#endif
    renderer = createRenderer(pdfDoc, page_part);

  // Check if the renderer is valid
  if (!renderer->isValid())
    qCritical() << tr("Creating renderer failed, default is")
                << preferences()->renderer;

  // Create threads.
  for (auto &thread : threads) {
    thread = new PixCacheThread(pdfDoc, renderer->pagePart(), this);
    connect(thread, &PixCacheThread::sendData, this, &PixCache::receiveData,
            Qt::QueuedConnection);
    connect(this, &PixCache::setPixCacheThreadPage, thread,
            &PixCacheThread::setNextPage, Qt::QueuedConnection);
  }
  mutex.unlock();
}

PixCache::~PixCache()
{
  debug_verbose(DebugFunctionCalls, "DELETING PixCache" << this);
  delete renderer;
  // TODO: correctly clean up threads!
  for (const auto &thread : std::as_const(threads)) thread->quit();
  for (const auto &thread : std::as_const(threads)) {
    thread->wait(10000);
    delete thread;
  }
  mutex.lock();
  clear();
  mutex.unlock();
}

void PixCache::clear()
{
  debug_verbose(DebugFunctionCalls, this);
  cache.clear();
  usedMemory = 0;
  region.first = preferences()->page;
  region.second = region.first;
}

const QPixmap PixCache::pixmap(const int page, qreal resolution)
{
  // Check if page number is valid.
  if (page < 0 || page >= pdfDoc->numberOfPages()) return QPixmap();

  if (resolution <= 0.) resolution = getResolution(page);

  // Try to return a page from cache.
  {
    mutex.lock();
    const auto it = cache.find(page);
    if (it != cache.cend() && it->second &&
        abs(it->second->getResolution() - resolution) <
            max_resolution_deviation) {
      QPixmap pix = it->second->pixmap();
      if (pix.isNull()) {
        usedMemory -= it->second->size();
        cache.erase(it);
      }
      mutex.unlock();
      return pix;
    }
    mutex.unlock();
  }

  // Check if the renderer is valid
  if (renderer == nullptr || !renderer->isValid()) {
    qCritical() << tr("Invalid renderer");
    return QPixmap();
  }

  debug_msg(DebugCache, "Rendering in main thread");
  const QPixmap pix = renderer->renderPixmap(page, resolution);

  if (pix.isNull()) {
    qCritical() << tr("Rendering page failed for (page, resolution) =") << page
                << resolution;
    return pix;
  }

  // Write pixmap to cache.
  auto png =
      std::unique_ptr<const PngPixmap>(new PngPixmap(pix, page, resolution));
  if (png == nullptr) {
    qWarning() << "Converting pixmap to PNG failed";
  } else {
    mutex.lock();
    usedMemory += png->size();
    const auto [it, inserted] = cache.try_emplace(page, nullptr);
    if (it->second) usedMemory -= it->second->size();
    it->second.swap(png);
    mutex.unlock();
  }
  return pix;
}

void PixCache::requestRenderPage(const int n)
{
  debug_verbose(DebugFunctionCalls, n << this);
  mutex.lock();
  if (!priority.contains(n) && cache.find(n) == cache.end()) priority.append(n);
  mutex.unlock();

  // Start rendering next page.
  if (thread() == QThread::currentThread()) startTimer(0);
}

void PixCache::pageNumberChanged(const int page)
{
  debug_verbose(DebugFunctionCalls, page << this);
  mutex.lock();
  // Update boundaries of the simply connected region.
  if (cache.find(page) == cache.end()) {
    // If current page is not yet in cache: make sure it is first in priority
    // queue.
    if (!priority.isEmpty() && priority.first() != page) {
      priority.removeOne(page);
      priority.push_front(page);
    }
    region.first = page;
    region.second = page;
    mutex.unlock();
    return;
  }

  // Make sure that current page is inside the region.
  if (region.first > page || region.second < page) {
    region.first = page - 1;
    region.second = page + 1;
  }

  // Extend the region as far as possible by searching for gaps.
  // Use that keys in QMap are sorted and can be accessed by iterators.
  auto left = std::make_reverse_iterator(cache.find(region.first));
  auto right = cache.find(region.second);
  while (left != cache.rend() && left->first == region.first) {
    ++left;
    --region.first;
  }
  while (right != cache.cend() && right->first == region.second) {
    ++right;
    ++region.second;
  }
  mutex.unlock();

  // Start rendering next page.
  if (thread() == QThread::currentThread()) startTimer(0);
}

int PixCache::limitCacheSize() noexcept
{
  debug_verbose(DebugFunctionCalls,
                usedMemory << maxMemory << cache.size() << maxNumber << this);
  // Check restrictions on memory usage and number of slides.
  if (maxMemory < 0 && maxNumber < 0) {
    // Check if all pages are already in memory.
    if (cache.size() == pdfDoc->numberOfPages()) return 0;
    return INT_MAX >> 1;
  }
  if (maxNumber == 0 || maxMemory == 0) {
    mutex.lock();
    clear();
    mutex.unlock();
    return 0;
  }

  const int pref_page = preferences()->page;
  mutex.lock();
  // Check if region is valid.
  if (region.first > region.second) {
    region.first = pref_page;
    region.second = pref_page;
  }

  // Number of really cached slides:
  // subtract number of currently active threads, for which cache contains a
  // nullptr.
  int cached_slides = cache.size();
  for (auto it = threads.cbegin(); it != threads.cend(); ++it)
    if ((*it) && (*it)->isRunning()) --cached_slides;
  if (cached_slides <= 0) {
    mutex.unlock();
    return INT_MAX >> 1;
  }

  // Calculate how many slides still fit in available memory
  int allowed_slides = INT_MAX >> 1;
  if (maxMemory > 0) {
    // Get currently used memory.
    // Get number of slides which still fit in available memory.
    if (usedMemory > 0 && cached_slides > 0)
      allowed_slides = (maxMemory - usedMemory) * cached_slides / usedMemory;
    else
      allowed_slides = threads.length();
    debug_verbose(DebugCache, "set allowed_slides"
                                  << usedMemory << cached_slides
                                  << allowed_slides << maxMemory
                                  << threads.length());
  }
  if (maxNumber > 0 && allowed_slides + cache.size() > maxNumber)
    allowed_slides = maxNumber - cache.size();

  // If threads.length() pages can be rendered without problems: return
  if (allowed_slides >= threads.length()) {
    mutex.unlock();
    return allowed_slides;
  }

  debug_msg(DebugCache, "prepared deleting from cache"
                            << usedMemory << maxMemory << allowed_slides
                            << cached_slides);

  // Deleting starts from first or last page in cache.
  // The aim is to shrink the cache to a simply connected region
  // around the current page.
  int first = cache.cbegin()->first;
  int last = cache.crbegin()->first;
  /// Cached page which should be removed.
  std::unique_ptr<const PngPixmap> remove;

  // Delete pages while allowed_slides is negative or too small to
  // allow updates.
  do {
    // If the set of cached pages is simply connected, includes the
    // current page, and lies mostly ahead of the current page,
    // then stop rendering to cache.
    if (((maxNumber < 0 || cache.size() <= maxNumber) &&
         (maxMemory < 0 || usedMemory <= maxMemory) && last > pref_page &&
         last - first <= cache.size() && 2 * last + 3 * first > 5 * pref_page)
        // the case cache.size() < 2 would lead to segfaults.
        || cache.size() < 2) {
      mutex.unlock();
      return 0;
    }

    // If more than 3/4 of the cached slides lie ahead of current page, clean up
    // last.
    if (last + 3 * first > 4 * pref_page) {
      const auto it = std::prev(cache.end());
      remove.swap(it->second);
      last = std::prev(cache.erase(it))->first;
    } else {
      const auto it = cache.begin();
      remove.swap(it->second);
      first = cache.erase(it)->first;
    }
    // Check if remove is nullptr (which means that a thread is just rendering
    // it).
    if (remove == nullptr) continue;
    debug_msg(DebugCache, "removing page from cache"
                              << usedMemory << allowed_slides << cached_slides
                              << remove->getPage());
    // Delete removed cache page and update memory size.
    usedMemory -= remove->size();
    --cached_slides;

    // Update allowed_slides
    if (usedMemory > 0 && cached_slides > 0) {
      allowed_slides = (maxMemory - usedMemory) * cached_slides / usedMemory;
      if (allowed_slides + cache.size() > maxNumber)
        allowed_slides = maxNumber - cache.size();
    } else
      allowed_slides = maxNumber - cache.size();

  } while (allowed_slides < threads.length() && cached_slides > 0);

  // Update boundaries of simply connected region
  if (first > region.first + 1) region.first = first - 1;
  if (last + 1 < region.second) region.second = last + 1;

  mutex.unlock();
  return allowed_slides;
}

int PixCache::renderNext()
{
  debug_verbose(DebugFunctionCalls, this);
  // Check if priority contains pages which are not yet rendered.
  int page;
  mutex.lock();
  while (!priority.isEmpty()) {
    page = priority.takeFirst();
    if (cache.find(page) == cache.end()) {
      mutex.unlock();
      return page;
    }
  }

  const int pref_page = preferences()->page;
  // Check if region is valid.
  if (region.first > region.second) {
    region.first = pref_page;
    region.second = region.first;
  }

  // Select region.first or region.second for rendering.
  while (true) {
    if (region.second + 3 * region.first > 4 * pref_page && region.first >= 0) {
      if (cache.find(region.first) == cache.end()) {
        mutex.unlock();
        return region.first--;
      }
      --region.first;
    } else {
      if (cache.find(region.second) == cache.end()) {
        mutex.unlock();
        return region.second++;
      }
      ++region.second;
    }
  }
}

void PixCache::timerEvent(QTimerEvent *event)
{
  debug_verbose(DebugFunctionCalls, event << this);
  killTimer(event->timerId());
  startRendering();
}

void PixCache::startRendering()
{
  debug_verbose(DebugCache | DebugFunctionCalls, "Start rendering" << this);
  // Clean up cache and check if there is enough space for more cached pages.
  int allowed_pages = limitCacheSize();
  if (allowed_pages <= 0) return;
  for (auto thread = threads.cbegin(); thread != threads.cend(); ++thread) {
    if (allowed_pages > 0 && *thread && !(*thread)->isRunning()) {
      const int page = renderNext();
      if (page < 0 || page >= pdfDoc->numberOfPages()) return;
      emit setPixCacheThreadPage(*thread, page, getResolution(page));
      --allowed_pages;
    }
  }
}

void PixCache::receiveData(const PngPixmap *data)
{
  debug_verbose(DebugFunctionCalls, data << this);
  if (QThread::currentThread() != this->thread()) {
    qCritical() << "Called PixCache::receiveData from wrong thread!";
    delete data;
    return;
  }

  // If a renderer failed, it should already have sent an error message.
  if (data == nullptr || data->isNull()) {
    delete data;
    return;
  }

  // Check if the received image is still compatible with the current
  // resolution.
  mutex.lock();
  const qreal good_resolution = getResolution(data->getPage());
  if (abs(good_resolution - data->getResolution()) > max_resolution_deviation) {
    // got page of wrong size, don't use it
    const auto it = cache.find(data->getPage());
    if (it != cache.end()) {
      if (it->second == nullptr) {
        cache.erase(it);
      } else if (abs(it->second->getResolution() - good_resolution) >
                 max_resolution_deviation) {
        usedMemory -= it->second->size();
        cache.erase(it);
      }
    }
    delete data;
  } else {
    usedMemory += data->size();
    const auto [it, inserted] = cache.try_emplace(data->getPage(), nullptr);
    if (it->second) usedMemory -= it->second->size();
    it->second.reset(data);
  }
  mutex.unlock();

  // Start rendering next page.
  startTimer(0);
}

qreal PixCache::getResolution(const int page) const
{
  debug_verbose(DebugFunctionCalls, page << this);
  // Get page size in points
  QSizeF pageSize = pdfDoc->pageSize(page);
  if (pageSize.isEmpty()) return -1.;
  if (renderer->pagePart() != FullPage) pageSize.rwidth() /= 2;
  switch (cacheMode) {
    case FitPage:
      if (pageSize.width() * frame.height() > pageSize.height() * frame.width())
        // page is too wide, determine resolution by x direction
        return frame.width() / pageSize.width();
      else
        // page is too high, determine resolution by y direction
        return frame.height() / pageSize.height();
    case FitWidth:
      return frame.width() / pageSize.width();
    case FitHeight:
      return frame.height() / pageSize.height();
    default:
      return -1.;
  }
}

void PixCache::updateFrame(const QSizeF &size)
{
  debug_verbose(DebugFunctionCalls, size << frame << this);
  if (frame != size && threads.length() > 0) {
    debug_msg(DebugCache, "update frame" << frame << size);
    mutex.lock();
    if ((cacheMode == FitWidth && frame.width() == size.width()) ||
        (cacheMode == FitHeight && frame.height() == size.height())) {
      frame = size;
    } else {
      frame = size;
      clear();
    }
    mutex.unlock();
  }
}

void PixCache::requestPage(const int page, const qreal resolution,
                           const bool cache_page)
{
  debug_verbose(DebugCache | DebugFunctionCalls,
                "requested page" << page << resolution << this);
  // Try to return a page from cache.
  {
    mutex.lock();
    const auto it = cache.find(page);
    debug_verbose(DebugCache,
                  "searched for page"
                      << page << (it == cache.cend())
                      << (it != cache.cend() && it->second != nullptr)
                      << (it == cache.cend()
                              ? -1024
                              : (it->second->getResolution() - resolution)));
    if (it != cache.cend() && it->second &&
        abs(it->second->getResolution() - resolution) <
            max_resolution_deviation) {
      QPixmap pix = it->second->pixmap();
      if (pix.isNull()) {
        usedMemory -= it->second->size();
        cache.erase(it);
      }
      mutex.unlock();
      emit pageReady(pix, page);
      return;
    }
    mutex.unlock();
  }
  // Check if page number is valid.
  if (page < 0 || page >= pdfDoc->numberOfPages()) return;

  // Render new page.
  // Check if the renderer is valid
  if (renderer == nullptr || !renderer->isValid()) {
    qCritical() << tr("Invalid renderer");
    return;
  }

  debug_msg(DebugCache, "Rendering page in PixCache thread" << this);
  const QPixmap pix = renderer->renderPixmap(page, resolution);

  if (pix.isNull()) {
    qCritical() << tr("Rendering page failed for (page, resolution) =") << page
                << resolution;
    return;
  }

  emit pageReady(pix, page);

  if (cache_page) {
    // Write pixmap to cache.
    std::unique_ptr<const PngPixmap> png(new PngPixmap(pix, page, resolution));
    if (png == nullptr)
      qWarning() << "Converting pixmap to PNG failed";
    else {
      mutex.lock();
      usedMemory += png->size();
      const auto [it, inserted] = cache.try_emplace(page, nullptr);
      if (it->second) usedMemory -= it->second->size();
      it->second.swap(png);
      debug_verbose(DebugCache, "writing page to cache" << page << usedMemory);
      mutex.unlock();
    }
  }

  // Start rendering next page.
  if (thread() == QThread::currentThread()) startTimer(0);
}

void PixCache::getPixmap(const int page, QPixmap &target, qreal resolution)
{
  debug_verbose(DebugFunctionCalls, page << resolution << this);
  target = pixmap(page, resolution);
}
