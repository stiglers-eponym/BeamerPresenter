#ifndef THUMBNAILTHREAD_H
#define THUMBNAILTHREAD_H

#include <QThread>
#include <QPixmap>

class AbstractRenderer;
class PdfDocument;
class ThumbnailButton;

/**
 * @brief Worker object for rendering thumbnails in own thread
 *
 * Created by ThumbnailWidget and moved to own thread, the ThumbnailThread
 * object renders thumbnail images and sends them to ThumbailWidget.
 * The images are connected to the ThumbnailButtons, at which they will
 * be shown.
 *
 * The images are not directly shown in the buttons from this thread,
 * because that should happen in the main thread.
 */
class ThumbnailThread : public QObject
{
    Q_OBJECT

    struct queue_entry {ThumbnailButton *button; qreal resolution; int page;};

    AbstractRenderer *renderer = NULL;
    const PdfDocument *document;
    QList<queue_entry> queue;

public:
    ThumbnailThread(const PdfDocument *document = NULL);
    ~ThumbnailThread()
    {thread()->deleteLater();}

public slots:
    void append(ThumbnailButton *button, qreal resolution, int page)
    {queue.append({button, resolution, page});}

    void renderImages();

signals:
    void sendThumbnail(ThumbnailButton *button, const QPixmap pixmap);
};

#endif // THUMBNAILTHREAD_H
