#ifndef PIXCACHE_H
#define PIXCACHE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QTimer>
#include "src/rendering/pngpixmap.h"
#include "src/rendering/pixcachethread.h"
#include "src/rendering/pdfdocument.h"
#include "src/preferences.h"
#include "src/enumerates.h"

#define MAX_RESOLUTION_DEVIATION 1e-5

/// Cache of compressed slides as PNG images.
/// This does the job of rendering slides to images and storing these images
/// in compressed cache.
class PixCache : public QObject
{
    Q_OBJECT

private:
    /// Map page numbers to cached PNG pixmaps.
    /// Pages which are currently being rendered are marked with a NULL here.
    QMap<int, const PngPixmap*> cache;

    /// List of pages which should be rendered next.
    QList<int> priority;

    /// Boundaries of simply connected region of cache containing current page.
    QPair<int,int> region = {INT_MAX, -1};

    /// Size in which the slides should be rendered.
    /// TODO: make sure this is updated.
    QSizeF frame;

    /// Amount of memory which should be used by this.
    float maxMemory = -1.f;

    /// Current size in bytes
    qint64 usedMemory = 0;

    /// Maximum number of slides in cache
    int maxNumber = -1;

    /// Threads used to render pages to cache.
    /// This will be an empty vector if the PDF has flexible page sizes.
    QVector<PixCacheThread*> threads;

    /// Own renderer for rendering in PixCache thread.
    AbstractRenderer *renderer;

    /// Pdf document.
    const PdfDocument *pdfDoc;

    /// Single shot, 0 duration timer for rendering pages.
    QTimer *renderCacheTimer {NULL};

    /// Check cache size and delete pages if necessary.
    /// Return estimated number of pages which still fit in cache.
    /// Return INT_MAX if cache is unlimited or empty.
    int limitCacheSize();

    /// Choose a page which should be rendered next.
    /// The page is then marked as "being rendered".
    /// This page must then also be rendered.
    int renderNext();

    /// Calculate resolution for given page number based on this->frame.
    /// Return resolution in pixels per point (72*dpi)
    qreal getResolution(const int page) const;

public:
    explicit PixCache(PdfDocument *doc, const int thread_number, const PagePart page_part = FullPage, QObject *parent = NULL);
    ~PixCache();

    /// Clear cache, delete all cached pages.
    void clear();

    /// Set maximum allowed bytes of memory used by this->cache.
    /// Clean up memory if necessary.
    void setMaxMemory(const float memory);

    /// Set maximum allowed number of cache slides.
    /// Clean up memory if necessary.
    void setMaxNumber(const int number);

    /// Get pixmap showing page n.
    const QPixmap pixmap(const int page) const;

    /// Get pixmap showing page n.
    /// The non-const function additionally writes a new pixmap to cache.
    const QPixmap pixmap(const int page);

    /// Total size of all cached pages in bytes
    qint64 getUsedMemory() const
    {return usedMemory;}

    /// Number of pixels per page (maximum)
    float getPixels() const
    {return frame.width() * frame.height();}

    void setScaledMemory(const float scale)
    {setMaxMemory(scale * frame.width() * frame.height());}

    /// Udate frame and clear cache if necessary.
    /// Cache will only be cleared if !threads.isEmpty(), because an empty
    /// thread vector indicates flexible slide size.
    void updateFrame(QSizeF const& size);

public slots:
    /// Create renderCacheTimer.
    /// This is an own function because it must be done in this thread.
    void init();

    /// Request rendering a page with high priority
    void requestPage(const int n, const qreal resolution);

    /// Request rendering a page with low priority
    void requestRenderPage(const int n);

    /// Start rendering the next page(s).
    void startRendering();

    /// Receive a PngPixmap from one of the threads.
    void receiveData(const PngPixmap *data);

    /// Update current page number.
    /// Update boundary of simply connected region of cached pages.
    /// This does not fully recalculate the region, but assumes that the
    /// currently saved region is indeed simply connected.
    void pageNumberChanged(const int page);

signals:
    /// Send out new page.
    void pageReady(const QPixmap pixmap, const int page) const;
};

#endif // PIXCACHE_H
