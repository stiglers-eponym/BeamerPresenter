// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PIXCACHE_H
#define PIXCACHE_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QSizeF>
#include <QVector>
#include <map>
#include <memory>

#include "src/config.h"
#include "src/enumerates.h"
#include "src/log.h"
#include "src/rendering/pngpixmap.h"

class QPixmap;
class QTimerEvent;
class PdfDocument;
class PixCacheThread;
class AbstractRenderer;

/**
 * @brief Cache of compressed slides as PNG images.
 *
 * This does the job of rendering slides to images and storing these images
 * in compressed cache.
 *
 * Objects of this class are moved to separate threads. These objects
 * should only be accessed via queued connections.
 */
class PixCache : public QObject
{
  Q_OBJECT

 public:
  enum CacheMode {
    FitPage,
    FitWidth,
    FitHeight,
  };

 private:
  static constexpr qreal max_resolution_deviation = 1e-5;

  /// Map page numbers to cached PNG pixmaps.
  /// Pages which are currently being rendered are marked with a nullptr here.
  /// std::map seems better than QMap for handling std::unique_ptr
  std::map<int, std::unique_ptr<const PngPixmap>> cache;

  /// Mutex to lock this thread.
  QMutex mutex;

  /// List of pages which should be rendered next.
  QList<int> priority;

  /// Boundaries of simply connected region of cache containing current page.
  QPair<int, int> region{INT_MAX, -1};

  /// Size in which the slides should be rendered.
  /// @todo make sure this is updated.
  QSizeF frame;

  /// Amount of memory which should be used by this.
  float maxMemory = -1.f;

  /// Current size in bytes
  qint64 usedMemory = 0;

  /// Maximum number of slides in cache
  int maxNumber = -1;

  /// Fixed width for cache in scroll mode
  CacheMode cacheMode = FitPage;

  /// Threads used to render pages to cache.
  /// This will be an empty vector if the PDF has flexible page sizes.
  QVector<PixCacheThread *> threads;

  /// Own renderer for rendering in PixCache thread.
  AbstractRenderer *renderer{nullptr};

  /// Pdf document.
  std::shared_ptr<const PdfDocument> pdfDoc;

  /// Check cache size and delete pages if necessary.
  /// Return estimated number of pages which still fit in cache.
  /// Return INT_MAX >> 1 if cache is unlimited or empty.
  int limitCacheSize() noexcept;

  /// Choose a page which should be rendered next.
  /// The page is then marked as "being rendered".
  /// This page must then also be rendered.
  int renderNext();

  /// Calculate resolution for given page number based on this->frame.
  /// Return resolution in pixels per point (72*dpi)
  qreal getResolution(const int page) const;

  /// Get pixmap showing page and write it to cache.
  const QPixmap pixmap(const int page, qreal resolution = -1.);

 protected:
  /// Timer event: stop the timer and start rendering next pixmap.
  void timerEvent(QTimerEvent *event) override;

 public:
  /// Constructor: only very basic initialization.
  /// Full initialization is done afterwards by init().
  explicit PixCache(const std::shared_ptr<PdfDocument> &doc,
                    const int thread_number,
                    const PagePart page_part = FullPage,
                    const CacheMode mode = FitPage,
                    QObject *parent = nullptr) noexcept;

  /// Destructor: Stop and clean up threads, delete renderer, clear content.
  ~PixCache();

  /// Set maximum allowed bytes of memory used by this->cache.
  /// Clean up memory if necessary.
  /// Not thread save!
  void setMaxMemory(const float memory) noexcept
  {
    debug_verbose(DebugFunctionCalls, memory << usedMemory << this);
    maxMemory = memory;
    if (memory < usedMemory && memory >= 0) limitCacheSize();
  }

  /// Set maximum allowed number of cache slides.
  /// Clean up memory if necessary.
  /// Not thread save!
  void setMaxNumber(const int number) noexcept
  {
    debug_verbose(DebugFunctionCalls, number << cache.size() << this);
    maxNumber = number;
    if (number < cache.size() && number >= 0) limitCacheSize();
  }

  /// Set cache mode, clear cache if mode changes.
  void setCacheMode(const CacheMode mode)
  {
    if (mode != cacheMode) {
      mutex.lock();
      cacheMode = mode;
      clear();
      mutex.unlock();
    }
  }

  /// Total size of all cached pages in bytes
  qint64 getUsedMemory() const noexcept { return usedMemory; }

  /// Number of pixels per page (maximum)
  float getPixels() const noexcept { return frame.width() * frame.height(); }

 public slots:
  /// Set memory based on scale factor (bytes per pixel).
  void setScaledMemory(const float scale)
  {
    debug_verbose(DebugFunctionCalls, scale << this);
    setMaxMemory(scale * frame.width() * frame.height());
  }

  /// Udate frame and clear cache if necessary.
  /// Cache will only be cleared if !threads.isEmpty(), because an empty
  /// thread vector indicates flexible slide size.
  void updateFrame(QSizeF const &size);

  /// Create renderCacheTimer.
  /// This is an own function because it must be done in this thread.
  void init();

  /// Clear cache, delete all cached pages.
  void clear();

  /// Request rendering a page with high priority
  /// May only be called in this object's thread.
  void requestPage(const int n, const qreal resolution,
                   const bool cache = true);

  /// Write pixmap representing *page* to *target*.
  /// Additionally write pixmap to cache if it needs to be created.
  void getPixmap(const int page, QPixmap &target, qreal resolution = -1.);

  /// Request rendering a page with low priority
  /// May only be called in this object's thread.
  void requestRenderPage(const int n);

  /// Start rendering the next page(s).
  void startRendering();

  /// Receive a PngPixmap from one of the threads.
  /// May only be called in this object's thread.
  void receiveData(const PngPixmap *data);

  /// Update current page number.
  /// Update boundary of simply connected region of cached pages.
  /// This does not fully recalculate the region, but assumes that the
  /// currently saved region is indeed simply connected.
  /// May only be called in this object's thread.
  void pageNumberChanged(const int slide, const int page);

 signals:
  /// Send out new page.
  void pageReady(const QPixmap pixmap, const int page);

  /// Notify target thread that it should work on given page.
  void setPixCacheThreadPage(const PixCacheThread *target,
                             const int page_number, const qreal res);
};

#endif  // PIXCACHE_H
