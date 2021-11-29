#ifndef THUMBNAILTHREAD_H
#define THUMBNAILTHREAD_H

#include <QPixmap>
#include "src/rendering/abstractrenderer.h"

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

    /// container of page, button and resolution as queued for rendering
    struct queue_entry
    {
        /// Button which should receive the thumbnail
        ThumbnailButton *button;
        /// thumbnail resolution
        qreal resolution;
        /// page index
        int page;
    };

    /// renderer, owned by this, created in constructor.
    AbstractRenderer *renderer = NULL;
    /// document, not owned by this.
    const PdfDocument *document;
    /// queue of pages/thumbnails which should be rendered
    QList<queue_entry> queue;

public:
    /// Constructor: create renderer if document is not NULL.
    ThumbnailThread(const PdfDocument *document = NULL);

    /// Destructor: delete renderer.
    ~ThumbnailThread()
    {delete renderer;}

public slots:
    /// Add entries to rendering queue.
    void append(ThumbnailButton *button, qreal resolution, int page)
    {queue.append({button, resolution, page});}

    /// Do the work: render thumbnails for the queued pages.
    void renderImages();

signals:
    /// Send thumbnail back to ThumbnailWidget, which sets the pixmap
    /// from the main thread.
    void sendThumbnail(ThumbnailButton *button, const QPixmap pixmap);
};

#endif // THUMBNAILTHREAD_H
