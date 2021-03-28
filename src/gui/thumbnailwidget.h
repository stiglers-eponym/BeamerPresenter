#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QScrollArea>
#include "src/gui/thumbnailbutton.h"

class PdfDocument;
class ThumbnailThread;

/**
 * @brief Widget showing thumbnail slides on grid layout in scroll area.
 *
 * TODO:
 *  don't change slide after touch scroll event
 *  clear when resizing?
 */
class ThumbnailWidget : public QScrollArea
{
    Q_OBJECT

public:
    enum
    {
        /// show one thumbnail per page label instead of per page
        SkipOverlays = 1 << 0,
    };

private:
    /// QObject for rendering. which is moved to an own thread.
    /// Communication to render_thread is almost exclusively done via the
    /// signal/slot mechanism since it lives in another thread.
    ThumbnailThread *render_thread = NULL;

    /// width of widget when thumbnails were rendered, in pixels.
    int ref_width = 0;
    /// number of columns
    unsigned char columns = 4;
    /// flags: currently only SkipOverlays.
    unsigned char _flags;

public:
    explicit ThumbnailWidget(QWidget *parent = NULL) : QScrollArea(parent)
    {setFocusPolicy(Qt::NoFocus);}

    ~ThumbnailWidget();

    /// Set number of columns.
    void setColumns(const unsigned char n_columns) noexcept
    {columns = n_columns;}

    unsigned char &flags() noexcept
    {return _flags;}

    /// (re)generate and show the thumbnails. This initializes render_thread
    /// if it does not exist yet. By default it takes document from
    /// preferences(). The function returns after (creating,) instructing
    /// and starting the rendering thread. Rendering is then done in
    /// background.
    void generate(const PdfDocument *document = NULL);

    bool hasHeightForWidth() const noexcept override
    {return true;}

    QSize sizeHint() const noexcept override
    {return {100, 200};}

public slots:
    /// generate thumbnails if necessary and select currenlty visible page.
    void showEvent(QShowEvent*) override;

    /// Receive thumbnail from render_thread and show it on button.
    void receiveThumbnail(ThumbnailButton *button, const QPixmap pixmap)
    {if (button) button->setPixmap(pixmap);}

signals:
    void sendNavigationSignal(int page);
    /// Tell render_thread to render page with resolution and associate it
    /// with button.
    void sendToRenderThread(ThumbnailButton *button, qreal resolution, int page);
    /// Tell render_thread to start rendering.
    void startRendering();
};

#endif // THUMBNAILWIDGET_H
