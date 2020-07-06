#ifndef PIXCACHETHREAD_H
#define PIXCACHETHREAD_H

#include <QThread>
#include "src/rendering/pngpixmap.h"
#include "src/rendering/abstractrenderer.h"

class PdfMaster;

/// Separate thread for rendering page pixmaps to (compressed) cache.
class PixCacheThread : public QThread
{
    Q_OBJECT

private:
    /// Renderer doing the main work.
    AbstractRenderer *renderer = nullptr;
    qreal resolution;
    int page;

public:
    PixCacheThread(QObject *parent = nullptr);

    /// Create a renderer based on preferences.
    /// Return true if successful and false if no renderer was created.
    bool initializeRenderer(const PdfMaster * const master);

    /// Do the work.
    void run() override;

    /// Set page number and resolution.
    /// Note that this is not thread save! But when starting to run the thread,
    /// the values are locally copied. That should make it nearly impossible to
    /// create problems using this function.
    void setNextPage(const int page_number, const qreal res);

signals:
    /// Send out the data.
    void sendData(const PngPixmap * const data);
};

#endif // PIXCACHETHREAD_H
