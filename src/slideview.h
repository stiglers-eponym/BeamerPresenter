#ifndef SLIDE_H
#define SLIDE_H

#include <QGraphicsView>
#include "src/gui/guiwidget.h"

class PixCache;
class SlideScene;

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView, GuiWidget
{
    Q_OBJECT

    /// PixCache instance responsible for rendering PDF pages.
    /// Not owned by this!
    PixCache* pixcache;
    /// Pixmap representing the current slide.
    QPixmap currentPixmap;

public:
    explicit SlideView(SlideScene *scene, PixCache *pixcache = nullptr, QWidget *parent = nullptr);

protected:
    /// Draw the slide to background (with correct resolution and from cache).
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

public slots:
    /// Inform this that the page number has changed.
    void pageChanged(const int page, const QSizeF &pagesize);
    /// Inform this that page is ready in pixcache.
    void pageReady(const int page, const qreal resolution);

signals:

};

#endif // SLIDE_H
