// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDE_H
#define SLIDE_H

#include <cstring>
#include <QGraphicsView>
#include "src/config.h"
#include "src/enumerates.h"

class QResizeEvent;
class PointingTool;
class PixCache;
class QWidget;
class MediaSlider;
class SlideScene;
class PixmapGraphicsItem;

namespace slide {
    struct MediaItem;
}

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
        MediaControls = 1 << 0,
        ShowAnimations = 1 << 1,
        ShowPointingTools = 1 << 2,
        ShowSelection = 1 << 3,
        ShowAll = 0xff,
    };

private:
    /// List of slides for video annotations in this view.
    QList<MediaSlider*> sliders;

    /// Currently waiting for page: INT_MAX if not waiting for any page.
    int waitingForPage = INT_MAX;

    /// Show slide transitions, multimedia, etc. (all not implemented yet).
    quint8 view_flags = ShowAll ^ MediaControls;

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

    /// Modifiable flags.
    quint8 &flags() noexcept
    {return view_flags;}

    /// read-only flags.
    const quint8 &flags() const noexcept
    {return view_flags;}

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
    void pageChanged(const int page, SlideScene* scene);

    /// Change page, but render page in this thread.
    /// Only required for fly transition.
    void pageChangedBlocking(const int page, SlideScene* scene);

    /// Inform this that page is ready in pixcache.
    void pageReady(const QPixmap pixmap, const int page);

    /// Draw magnifier to painter. tool should have BasicTool Magnifier, but this is not checked.
    void showMagnifier(QPainter *painter, const PointingTool *tool) noexcept;
    /// Draw pointer to painter. tool should have BasicTool Pointer, but this is not checked.
    void showPointer(QPainter *painter, const PointingTool *tool) noexcept;
    /// Draw torch to painter. tool should have BasicTool Torch, but this is not checked.
    void showTorch(QPainter *painter, const PointingTool *tool) noexcept;
    /// Draw circle representing eraser.
    void showEraser(QPainter *painter, const PointingTool *tool) noexcept;

    /// Draw pointing tools in foreground.
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /// Add a slider for a video item
    void addMediaSlider(const slide::MediaItem &media);

    /// Prepare a slide transition: render current view to transitionItem.
    void prepareTransition(PixmapGraphicsItem *transitionItem);

    /**
     * @brief prepare fly transition by writing difference of current slide and old pixmap to target.
     * @param outwards: bool, define whether transition is outwards or inwards
     * @param old: contains pixmaps of the old page.
     * @param target: write the result here. old == target is allowed.
     */
    void prepareFlyTransition(const bool outwards, const PixmapGraphicsItem *old, PixmapGraphicsItem *target);

signals:
    /// Inform cache that page is required.
    /// Resolution is given in pixels per point (dpi/72).
    void requestPage(const int page, const qreal resolution, const bool cache_page = true) const;

    /// Send key event to Master.
    void sendKeyEvent(QKeyEvent *event) const;

    /// Send an action (from a gesture) to master.
    void sendAction(Action action) const;

    /// Inform cache that widget has been resized.
    void resizeCache(const QSizeF &size) const;

    /// Draw foreground: pointing tools
    void drawSceneForeground(QPainter *painter, const QRectF &rect);

    /// Get pixmap from pixcache. This function renders the pixmap in the
    /// main thread and directly writes the result to pixmap.
    void getPixmapBlocking(const int page, QPixmap *pixmap, qreal resolution);
};

#endif // SLIDE_H
