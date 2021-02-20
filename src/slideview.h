#ifndef SLIDE_H
#define SLIDE_H

#include <QSlider>
#include <QGraphicsView>
#include <QResizeEvent>
#include "src/drawing/pointingtool.h"
#include "src/rendering/pixcache.h"
#include "src/preferences.h"
#include "src/slidescene.h"

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView
{
    Q_OBJECT

public:
    /// Flags defining what is shown. Most of these are not implemented yet,
    /// some may never get implemented.
    enum ViewFlag
    {
        LoadVideos = 1 << 0,
        AutoplayVideos = 1 << 1,
        LoadSounds = 1 << 2,
        AutoplaySounds = 1 << 3,
        AutoplayAnything = AutoplayVideos | AutoplaySounds,
        LoadAnyMedia = 0xf,
        MediaControls = 1 << 4,
        ShowAnimations = 1 << 5,
        ShowTransitions = 1 << 6,
        ShowDrawings = 1 << 7,
        ShowPointingTools = 1 << 8,
        ShowAll = 0xffff,
    };

private:
    /// Pixmap representing the current slide.
    QPixmap currentPixmap;
    /// Enlarged pixmap of current slide (for magnifier).
    QPixmap enlargedPixmap;

    QList<QSlider*> sliders;

    /// Currently waiting for page: INT_MAX if not waiting for any page,
    /// (-page-1) if waiting for enlarged page.
    int waitingForPage = INT_MAX;

    /// Show slide transitions, multimedia, etc. (all not implemented yet).
    int flags = ShowAll ^ MediaControls;

public:
    /// Constructor: initialize and connect a lot.
    explicit SlideView(SlideScene *scene, PixCache *cache = NULL, QWidget *parent = NULL);

    ~SlideView() noexcept;

    /// Preferred height of the layout depends on its width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Preferred height at given width based on scene aspect ratio.
    int heightForWidth(int width) const noexcept override;

    /// Size hint: scene size in points.
    QSize sizeHint() const noexcept override
    {return (3*scene()->sceneRect().size()).toSize();}

    /// Convert a position in widget coordinate (pixels) to scene coordinates
    /// (points).
    const QPointF mapToScene(const QPointF& pos) const;

    void setFlag(const int flag) noexcept
    {flags |= flag;}

    void unsetFlag(const int flag) noexcept
    {flags &= ~flag;}

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

    /// draw pointing tools in foreground.
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void addMediaSlider(const SlideScene::VideoItem &video);

signals:
    /// Inform cache that page is required.
    /// Resolution is given in pixels per point (dpi/72).
    void requestPage(const int page, const qreal resolution, const bool cache_page = true) const;

    /// Send key event to Master.
    void sendKeyEvent(QKeyEvent *event) const;

    /// Inform cache that widget has been resized.
    void resizeCache(const QSizeF &size) const;

    void drawSceneForeground(QPainter *painter, const QRectF &rect);
};

#endif // SLIDE_H
