// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDE_H
#define SLIDE_H

#include <QGraphicsView>
#include <cstring>
#include <memory>

#include "src/config.h"
#include "src/enumerates.h"
#include "src/media/mediaslider.h"

class QResizeEvent;
class QGestureEvent;
class PointingTool;
class PixCache;
class QWidget;
class SlideScene;
class PixmapGraphicsItem;
class MediaItem;

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView
{
  Q_OBJECT

 public:
  /// Flags defining what is shown. Most of these are not implemented yet,
  /// some may never get implemented.
  enum ViewFlag {
    MediaControls = 1 << 0,
    ShowAnimations = 1 << 1,
    ShowPointingTools = 1 << 2,
    ShowSelection = 1 << 3,
    ShowAll = 0xff,
  };
  Q_DECLARE_FLAGS(ViewFlags, ViewFlag);
  Q_FLAG(ViewFlags);

 private:
  qreal resolution = 0.0;

  /// List of slides for video annotations in this view.
  std::list<std::unique_ptr<MediaSlider>> sliders;

  /// Currently waiting for page: INT_MAX if not waiting for any page.
  int waitingForPage = INT_MAX;

  /// Show slide transitions, multimedia, etc. (all not implemented yet).
  ViewFlags view_flags = {ShowAll ^ MediaControls};

  /// Send request for rendering page with resolution increased by zoom relative
  /// to normal view.
  void requestScaledPage(const qreal zoom);

 protected:
  /// Handle gesture events. Currently, this handles swipe and pinch gestures
  bool handleGestureEvent(QGestureEvent *event);

 public:
  /// Constructor: initialize and connect a lot.
  explicit SlideView(SlideScene *scene, const PixCache *cache = nullptr,
                     QWidget *parent = nullptr);

  /// Trivial destructor.
  ~SlideView() { sliders.clear(); }

  /// Preferred height of the layout depends on its width.
  bool hasHeightForWidth() const noexcept override { return true; }

  /// Preferred height at given width based on scene aspect ratio.
  int heightForWidth(int width) const noexcept override;

  /// Size hint: scene size aspect ratio normalized to long side = 2048
  QSize sizeHint() const noexcept override;

  /// Convert a position in widget coordinate (pixels) to scene coordinates
  /// (points).
  const QPointF mapToScene(const QPointF &pos) const;

  /// Modifiable flags.
  ViewFlags &flags() noexcept { return view_flags; }

  /// Read-only flags.
  const ViewFlags &flags() const noexcept { return view_flags; }

  /// Set zoom relative to normal size. If render is true, request to render the
  /// page with adjusted resolution.
  void setZoom(const qreal zoom, const bool render = true)
  {
    if (render) requestScaledPage(zoom);
    const qreal rel_scale = zoom / transform().m11() * resolution;
    scale(rel_scale, rel_scale);
  }

 protected slots:
  /// Handle tablet events. The tablet events are mainly handed over to
  /// the scene.
  bool event(QEvent *event) override;

  /// Resize widget. Resizes cache.
  void resizeEvent(QResizeEvent *event) override;

  /// Handle key events: send them to Master.
  void keyPressEvent(QKeyEvent *event) override;

 public slots:
  /// Inform this that the page number has changed.
  void pageChanged(const int page, SlideScene *scene);

  /// Change page, but render page in this thread.
  /// Only required for fly transition.
  void pageChangedBlocking(const int page, SlideScene *scene);

  /// Inform this that page is ready in pixcache.
  void pageReady(const QPixmap pixmap, const int page);

  /// Draw magnifier to painter. tool should have BasicTool Magnifier, but this
  /// is not checked.
  void showMagnifier(QPainter *painter,
                     std::shared_ptr<const PointingTool> tool) noexcept;
  /// Draw pointer to painter. tool should have BasicTool Pointer, but this is
  /// not checked.
  void showPointer(QPainter *painter,
                   std::shared_ptr<const PointingTool> tool) noexcept;
  /// Draw torch to painter. tool should have BasicTool Torch, but this is not
  /// checked.
  void showTorch(QPainter *painter,
                 std::shared_ptr<const PointingTool> tool) noexcept;
  /// Draw circle representing eraser.
  void showEraser(QPainter *painter,
                  std::shared_ptr<const PointingTool> tool) noexcept;

  /// Draw pointing tools in foreground.
  void drawForeground(QPainter *painter, const QRectF &rect) override;

  /// Add a slider for a video item
  void addMediaSlider(const std::shared_ptr<MediaItem> media);

  /// Prepare a slide transition: render current view to transitionItem.
  void prepareTransition(PixmapGraphicsItem *transitionItem);

  /**
   * @brief prepare fly transition by writing difference of current slide and
   * old pixmap to target.
   * @param outwards: bool, define whether transition is outwards or inwards
   * @param old: contains pixmaps of the old page.
   * @param target: write the result here. old == target is allowed.
   */
  void prepareFlyTransition(const bool outwards, const PixmapGraphicsItem *old,
                            PixmapGraphicsItem *target);

 signals:
  /// Inform cache that page is required.
  /// Resolution is given in pixels per point (dpi/72).
  void requestPage(const int page, const qreal resolution,
                   const bool cache_page = true) const;

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
  void getPixmapBlocking(const int page, QPixmap &pixmap, qreal resolution);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SlideView::ViewFlags);

#endif  // SLIDE_H
