#ifndef PIXCACHETHREAD_H
#define PIXCACHETHREAD_H

#include <QThread>
#include "pngpixmap.h"

/// Separate thread for rendering page pixmaps to (compressed) cache.
class PixCacheThread : public QThread
{
    Q_OBJECT

private:
    const PngPixmap* data;
    int page;

public:
    PixCacheThread(QObject *parent = nullptr);

    /// Do the work.
    void run() override;

    /// Note that this is not thread save! But when starting to run the thread,
    /// the value of page is locally copied. That should make it nearly
    /// impossible to create problems using this function.
    void setPageNumber(const int n) {page = n;}

signals:
    /// Send out the data.
    void sendData(const PngPixmap * const data);
};

#endif // PIXCACHETHREAD_H
