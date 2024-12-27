// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <map>
#include <memory>

#include "src/config.h"
#if (QT_VERSION_MAJOR >= 6)
#include <QAudioOutput>
#ifdef USE_WEBCAMS
#include <QCamera>
#include <QMediaCaptureSession>
#endif
#endif
#include "src/drawing/selectionrectitem.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/drawing/tool.h"
#include "src/enumerates.h"
#include "src/gui/toolpropertybutton.h"
#include "src/media/mediaitem.h"
#include "src/rendering/pdfdocument.h"

class QAbstractAnimation;
class QGraphicsItem;
class QGraphicsRectItem;
class PdfMaster;
class DrawTool;
class TextTool;
class DragTool;
class QFont;
class PointingTool;
class SelectionTool;
class PathContainer;
class PixmapGraphicsItem;
class QPropertyAnimation;
class QXmlStreamReader;
class AbstractGraphicsPath;

namespace drawHistory
{
struct DrawToolDifference;
struct TextPropertiesDifference;
struct ZValueChange;
struct Step;
}  // namespace drawHistory

/**
 * @brief QGraphicsScene for a presentation slide.
 *
 * Handles drawing events and links. Only instances of SlideView and no
 * usual QGraphicsViews may show instances of SlideScene.
 */
class SlideScene : public QGraphicsScene
{
  Q_OBJECT

 public:
  /// Settings for slide scenes, which apply to all views connected to the
  /// scene.
  enum SlideFlag {
    LoadMedia = 1 << 0,
    CacheVideos = 1 << 1,
    AutoplayVideo = 1 << 2,
    AutoplaySounds = 1 << 3,
    MuteSlide = 1 << 4,
    ShowTransitions = 1 << 5,
    ShowDrawings = 1 << 6,
    ShowSearchResults = 1 << 7,
    UnrenderedZoom = 1 << 8,
    Default = 0xff,
  };
  Q_DECLARE_FLAGS(SlideFlags, SlideFlag);
  Q_FLAGS(SlideFlags);

 private:
  /// settings for this slide scene.
  SlideFlags slide_flags = SlideFlag::Default;

  /// timer for rendering after zoom
  QTimer *zoom_timer{nullptr};

  /// Path which is currently being drawn.
  /// nullptr if currenty no path is drawn.
  QGraphicsItem *currentlyDrawnItem{nullptr};

  /// Group of path segments forming the currently drawn path.
  /// This collection of segments is directly made visible and gets deleted
  /// when drawing the path is completed and the path itself is shown
  /// instead.
  QGraphicsItemGroup *currentItemCollection{nullptr};

  /// Searched results which should be highlighted
  /// This item gets many rectangles as child objects.
  QGraphicsItemGroup *searchResults{nullptr};

  /// Graphics item representing the PDF page background.
  PixmapGraphicsItem *pageItem{nullptr};
  /// Graphics item required during a page transition, usually
  /// represents the old page.
  PixmapGraphicsItem *pageTransitionItem{nullptr};
  /// Slide transition type
  SlideTransition::Type transitionType{SlideTransition::Replace};

  /// List of (cached or active) video items.
  QList<std::shared_ptr<MediaItem>> mediaItems;

  /// PDF document, including drawing paths.
  /// This is const, all data sent to master should be send via signals.
  std::shared_ptr<const PdfMaster> master;

  /// Animation for slide transitions. Should be nullptr while no
  /// slide transition is active.
  QAbstractAnimation *animation{nullptr};

  /// Shift of currently shown page relative to the current presentation
  /// slide. Overlay specifications are stored in the nonzero bits of
  /// FirstOverlay and LastOverlay.
  PageShift shift = {0, ShiftOverlays::NoOverlay};

  /// Currently visible page.
  int page = 0;

  /// Fixed height for view in scroll mode
  qreal fixed_page_height = -1.0;

  /// Page part shown in this scene.
  const PagePart page_part;

  /// Page (part) size (pt)
  QSizeF page_size;

  /// Zoom factor
  qreal zoom = 1.0;

  /// Bounding rect of all currently selected items.
  SelectionRectItem selection_bounding_rect;

  /// Selection tool that is tempoarily created when clicking on selection
  /// rectangle handles.
  std::shared_ptr<SelectionTool> tmp_selection_tool{nullptr};

  /// Start slide transition.
  void startTransition(const int newpage, const SlideTransition &transition);

  /// Search video annotation in cache and create + add it to cache if
  /// necessary.
  std::shared_ptr<MediaItem> &getMediaItem(
      std::shared_ptr<MediaAnnotation> annotation, const int page);

  /// Create an animation object for a split slide transition.
  void createSplitTransition(const SlideTransition &transition,
                             PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a blinds slide transition.
  void createBlindsTransition(const SlideTransition &transition,
                              PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a box slide transition.
  void createBoxTransition(const SlideTransition &transition,
                           PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a wipe slide transition.
  void createWipeTransition(const SlideTransition &transition,
                            PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a fly slide transition.
  void createFlyTransition(const SlideTransition &transition,
                           PixmapGraphicsItem *pageTransitionItem,
                           PixmapGraphicsItem *oldPage);

  /// Create an animation object for a push slide transition.
  void createPushTransition(const SlideTransition &transition,
                            PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a cover slide transition.
  void createCoverTransition(const SlideTransition &transition,
                             PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for an uncover slide transition.
  void createUncoverTransition(const SlideTransition &transition,
                               PixmapGraphicsItem *pageTransitionItem);

  /// Create an animation object for a face slide transition.
  void createFadeTransition(const SlideTransition &transition,
                            PixmapGraphicsItem *pageTransitionItem);

 public:
  /// Constructor: initialize master, page_part, and QGraphisScene.
  /// Connect signals.
  explicit SlideScene(std::shared_ptr<const PdfMaster> master,
                      const PagePart part = FullPage,
                      QObject *parent = nullptr);

  /// Destructor: delete all graphics items.
  ~SlideScene();

  /// Get page size of current slide.
  const QSizeF &pageSize() const noexcept { return page_size; }

  /// Get current zoom factor.
  qreal getZoom() const noexcept { return zoom; }

  /// Reset scene rect to view the whole slide.
  void resetView();

  /** Set zoom factor.
   * @param new_zoom zoom factor relative to default view
   * @param reference anchor point that remains fixed in the view while zooming
   * (in scene coordinates)
   * @param render if true, views will render the slide with adjusted resolution
   */
  void setZoom(const qreal new_zoom, const QPointF reference,
               const bool render = true);
  /** Set zoom factor, anchor point is the center of the view.
   * @param new_zoom zoom factor relative to default view
   * @param render if true, views will render the slide with adjusted resolution
   */
  void setZoom(const qreal new_zoom, const bool render = true);

  void renderZoom();

  /// modifiable slide flags.
  SlideFlags &flags() noexcept { return slide_flags; }

  /// read-only slide flags
  const SlideFlags &flags() const noexcept { return slide_flags; }

  /// video items on all slides (cached or active).
  QList<std::shared_ptr<MediaItem>> &getMedia() noexcept { return mediaItems; }

  /// Get current page item (the pixmap graphics item showing the current page)
  PixmapGraphicsItem *pageBackground() const noexcept { return pageItem; }

  /// Set shift (number of pages and overlays).
  void setPageShift(const PageShift relative_shift) noexcept
  {
    shift = relative_shift;
  }

  /// Get PdfMaster master.
  std::shared_ptr<const PdfMaster> getPdfMaster() noexcept { return master; }

  /// Currently visible page.
  int getPage() const noexcept { return page; }

  /// Shift (number of pages and overlays).
  PageShift getShift() const noexcept { return shift; }

  /// return page_part.
  PagePart pagePart() const noexcept { return page_part; }

  /// Set fixed height for scroll mode.
  void setFixedHeight(const qreal height) noexcept
  {
    fixed_page_height = height;
  }

  /// Set fixed height for scroll mode based on aspect ratio of first page.
  void setFixedAspect(const qreal aspect);

  /// Handle tablet move event, mainly for drawing.
  /// Called from SlideView.
  void tabletMove(const QPointF &pos, const Tool::InputDevices device,
                  const qreal pressure)
  {
    handleEvents(device | Tool::UpdateEvent, {pos}, QPointF(), pressure);
  }

  /// Handle tablet press event, mainly for drawing.
  /// Called from SlideView.
  void tabletPress(const QPointF &pos, const Tool::InputDevices device,
                   const qreal pressure)
  {
    handleEvents(device | Tool::StartEvent, {pos}, QPointF(), pressure);
  }

  /// Handle tablet release event, mainly for drawing.
  /// Called from SlideView.
  void tabletRelease(const QPointF &pos, const Tool::InputDevices device,
                     const qreal pressure = 0.)
  {
    handleEvents(device | Tool::StopEvent, {pos}, QPointF(), pressure);
  }

  /// Update geometry in preparation of navigation event.
  /// newpage does not include the shift.
  void prepareNavigationEvent(const int newslide, const int newpage);

  /// Receive navigation event from PdfMaster.
  /// The given page already includes the shift.
  void navigationEvent(const int newslide, const int newpage,
                       SlideScene *newscene = nullptr);

  /// Start handling draw and erase events.
  void startInputEvent(std::shared_ptr<const DrawTool> tool, const QPointF &pos,
                       const float pressure = 1.);

  /// Handle draw and erase events.
  void stepInputEvent(std::shared_ptr<const DrawTool> tool, const QPointF &pos,
                      const float pressure = 1.);

  /// Finish handling draw and erase events.
  bool stopInputEvent(std::shared_ptr<const DrawTool> tool);

  /// Check if currently text is beeing edited.
  bool isTextEditing() const noexcept
  {
    return focusItem() && focusItem()->type() == TextGraphicsItem::Type;
  }

  /// Copy selection to clipboard.
  void copyToClipboard() const;

  /// Remove selection.
  void removeSelection();

  /// Paste from clipboard.
  void pasteFromClipboard();

  /// Check if a selection rect handle is clicked, start selection if necessary.
  bool maybeStartSelectionEvent(const QPointF &pos,
                                const Tool::InputDevices device) noexcept;

  /// Initialize tmp_selection_tool with current selection.
  void initTmpSelectionTool(const Tool::InputDevices device) noexcept;

 protected:
  /**
   * @brief handle pointing device events.
   *
   * Handle mouse, touch and tablet input events for drawing,
   * highlighting and pointing.
   * Different types of pointing device events are unified and passed on to
   * handleEvents.
   */
  bool event(QEvent *event) override;

  /// Clicked on slide without draw tool (follow link, play video, ...)
  bool noToolClicked(const QPointF &pos, const QPointF &startpos = QPointF());

  /// Handle events from different pointing devices.
  bool handleEvents(const Tool::InputDevices device, const QList<QPointF> &pos,
                    const QPointF &start_pos, const float pressure);

  /// Helper function for handleEvents: draw tool events
  void handleDrawEvents(std::shared_ptr<const DrawTool> tool,
                        const Tool::InputDevices device,
                        const QList<QPointF> &pos, const float pressure);
  /// Helper function for handleEvents: pointing tool events
  void handlePointingEvents(std::shared_ptr<PointingTool> tool,
                            const Tool::InputDevices device,
                            const QList<QPointF> &pos);
  /// Helper function for handleEvents: selection tool events
  void handleSelectionEvents(std::shared_ptr<SelectionTool> tool,
                             const Tool::InputDevices device,
                             const QList<QPointF> &pos,
                             const QPointF &start_pos);
  /// Helper function for handleEvents: text tool events
  bool handleTextEvents(std::shared_ptr<const TextTool> tool,
                        const Tool::InputDevices device,
                        const QList<QPointF> &pos);
  bool handleDragView(std::shared_ptr<DragTool> tool,
                      const Tool::InputDevices device,
                      const QList<QPointF> &pos, const QPointF &start_pos);
  /// Handle selection start events (only called from handleEvents().
  void handleSelectionStartEvents(std::shared_ptr<SelectionTool> tool,
                                  const QPointF &pos);
  /// Handle selection stop events (only called from handleEvents().
  void handleSelectionStopEvents(std::shared_ptr<SelectionTool> tool,
                                 const QPointF &pos, const QPointF &start_pos);

 public slots:
  /// Stop drawing and convert just drawn path to regular path.
  void stopDrawing();

  /// Receive and handle action.
  void receiveAction(const Action action);

  /// Load and show all media annotations on this slide.
  void loadMedia(const int page);

  /// Load media for given page to cache.
  void cacheMedia(const int page);

  /// Tasks done after rendering: load media for next page to cache.
  void postRendering();

  /// Tell views to create sliders.
  void createSliders() const;

  /// End slide transition.
  void endTransition();

  /// Send transition step notification to views.
  void transitionStep()
  {
    invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
  }

  /// Play all media on current slide.
  void playMedia() const;
  /// Pause all media on current slide.
  void pauseMedia() const;
  /// If anything is playing, pause everything. Otherwise play everything.
  void playPauseMedia() const;

  /// Update selection_bounding_rect.
  void updateSelectionRect() noexcept;

  /// Update tool, change selected items if necessary.
  void toolChanged(std::shared_ptr<Tool> tool) noexcept;

  /// Update tool properties for selected items.
  void toolPropertiesChanged(const tool_variant &properties) noexcept;

  /// Show search results taken from PdfMaster.
  void updateSearchResults();

 signals:
  /// Send navigation event to views.
  /// Here page is already adapted to shift.
  void navigationToViews(const int page, SlideScene *scene);

  /// Send a navigation signal (to master).
  void navigationSignal(const int slide, const int page);

  /// Send action (to master).
  void sendAction(const Action action);

  /// Tell views to clear background.
  void clearViews();

  /// Send new path to PdfMaster.
  void sendNewPath(PPage ppage, QGraphicsItem *item);

  /// Send transformations for QGraphicsItems to PdfMaster.
  void sendHistoryStep(
      PPage ppage, std::map<QGraphicsItem *, QTransform> *transforms,
      std::map<AbstractGraphicsPath *, drawHistory::DrawToolDifference> *tools,
      std::map<TextGraphicsItem *, drawHistory::TextPropertiesDifference>
          *texts);

  /// Replace old path by new path in a single drawing history step.
  void replacePath(PPage ppage, QGraphicsItem *olditem, QGraphicsItem *newitem);

  /// Add new paths in single history step.
  void sendAddPaths(PPage ppage, const QList<QGraphicsItem *> &paths);

  /// Remove paths in single history step.
  void sendRemovePaths(PPage ppage, const QList<QGraphicsItem *> &paths);

  /// Tell master that transition has ended.
  void finishTransition();

  /// Get path container for given page, create one if cummulative
  /// drawing requires that.
  void requestNewPathContainer(PathContainer **container, PPage ppage);

  /// Get path container for given page, create one if it does not exist.
  void createPathContainer(PathContainer **container, PPage ppage);

  /// Notify master that there are unsaved changes.
  void newUnsavedDrawings();

  /// Bring given items to foreground and add history step.
  void bringToForeground(PPage ppage,
                         const QList<QGraphicsItem *> &to_foreground);
  /// Bring given items to background and add history step.
  void bringToBackground(PPage ppage,
                         const QList<QGraphicsItem *> &to_background);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SlideScene::SlideFlags);

/**
 * Read an SVG image. Create a GraphicsPictureItem which is added to target.
 * @param data read SVG image from this QByteArray
 * @param target add the GraphicsPictureItem constructed from the SVG image to
 * this list
 */
void readFromSVG(const QByteArray &data, QList<QGraphicsItem *> &target);

/**
 * Read a pixel-based image. Create a GraphicsPictureItem which is added to
 * target.
 * @param data read PNG image from this QByteArray
 * @param target add the GraphicsPictureItem constructed from the image to this
 * list
 */
void readFromPixelImage(const QByteArray &data, QList<QGraphicsItem *> &target,
                        const char *format);

/**
 * write items to an SVG image
 * @param data write result here
 * @param source take items from this list
 * @param rect sets viewBox of the SVG image
 */

void writeToSVG(QByteArray &data, const QList<QGraphicsItem *> &source,
                const QRectF &rect);
/**
 * write items to a pixel-based image
 * @param data write result here
 * @param source take items from this list
 * @param rect sets view box of the scene, which will be drawn to the image
 */
void writeToPixelImage(QByteArray &data, const QList<QGraphicsItem *> &source,
                       const QRectF &rect, const qreal resolution = 1.,
                       const char *format = "PNG");

#endif  // SLIDESCENE_H
