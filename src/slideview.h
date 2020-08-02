#ifndef SLIDE_H
#define SLIDE_H

#include <QGraphicsView>
#include <QResizeEvent>
#include "src/gui/guiwidget.h"

class PixCache;
class SlideScene;

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView, public GuiWidget
{
    Q_OBJECT

    /// Pixmap representing the current slide.
    QPixmap currentPixmap;
    /// Currently waiting for page (-1 if not waiting for any page):
    int waitingForPage = -1;

public:
    explicit SlideView(SlideScene *scene, PixCache *cache = nullptr, QWidget *parent = nullptr);
    const QSizeF preferedSize(QSizeF const& parent_size) const override;
    void setWidth(const qreal width) override
    {setMaximumWidth(width);}
    void setHeight(const qreal height) override
    {setMaximumHeight(height);}
    const QPointF mapToScene(const QPointF& pos) const;

protected:
    /// Draw the slide to background (with correct resolution and from cache).
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

protected slots:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Inform this that the page number has changed.
    void pageChanged(const int page, const QSizeF &pageSize);
    /// Inform this that page is ready in pixcache.
    void pageReady(const QPixmap pixmap, const int page);

signals:
    void requestPage(const int page, const qreal resolution) const;
    void sendKeyEvent(QKeyEvent *event) const;
    void resizeCache(const QSizeF &size) const;
    void tabletMoveEvent(const QPointF &pos, const QTabletEvent *event) const;
    void tabletPressEvent(const QPointF &pos, const QTabletEvent *event) const;
    void tabletReleaseEvent(const QPointF &pos, const QTabletEvent *event) const;
};

#endif // SLIDE_H
