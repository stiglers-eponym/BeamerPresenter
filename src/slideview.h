#ifndef SLIDE_H
#define SLIDE_H

#include <QDebug>
#include <QGraphicsView>
#include <QResizeEvent>
#include "src/drawing/pointingtool.h"

class PixCache;
class SlideScene;

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView
{
    Q_OBJECT

    /// Pixmap representing the current slide.
    QPixmap currentPixmap;

    /// Currently waiting for page (-1 if not waiting for any page).
    int waitingForPage = -1;

public:
    /// Constructor: initialize and connect a lot.
    explicit SlideView(SlideScene *scene, PixCache *cache = NULL, QWidget *parent = NULL);

    /// Preferred height of the layout depends on its width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Preferred height at given width based on scene aspect ratio.
    int heightForWidth(int width) const noexcept override;

    /// Size hint: scene size in points.
    QSize sizeHint() const noexcept override
    {return scene()->sceneRect().toAlignedRect().size();}

    /// Convert a position in widget coordinate (pixels) to scene coordinates
    /// (points).
    const QPointF mapToScene(const QPointF& pos) const;

protected:
    /// Draw the slide to background (with correct resolution and from cache).
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

protected slots:
    /// Handle tablet events. The tablet events are mainly handed over to
    /// the scene.
    bool event(QEvent* event) override;

    /// Resize widget. Resizes cache.
    void resizeEvent(QResizeEvent *event) override;

    /// Handle key events: send them to Master.
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Inform this that the page number has changed.
    /// pageSize is given in points.
    void pageChanged(const int page, SlideScene* scene);

    /// Inform this that page is ready in pixcache.
    void pageReady(const QPixmap pixmap, const int page);

    /// Set currentPixmap to empty QPixmap.
    void clearBackground() noexcept
    {currentPixmap = QPixmap();}

    /// draw Magnifier
    void showMagnifier(QPainter *painter, const PointingTool *tool);

    void drawForeground(QPainter *painter, const QRectF &rect) override;

signals:
    /// Inform cache that page is required.
    /// Resolution is given in pixels per point (dpi/72).
    void requestPage(const int page, const qreal resolution) const;

    /// Send key event to Master.
    void sendKeyEvent(QKeyEvent *event) const;

    /// Inform cache that widget has been resized.
    void resizeCache(const QSizeF &size) const;

    void drawSceneForeground(QPainter *painter, const QRectF &rect);
};

#endif // SLIDE_H
