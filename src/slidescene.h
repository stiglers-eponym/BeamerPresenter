// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <set>
#include <map>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QTabletEvent>
#include <QGraphicsScene>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/drawing/tool.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/drawing/selectionrectitem.h"

class QAbstractAnimation;
class QGraphicsItem;
class QGraphicsRectItem;
class QGraphicsVideoItem;
class PdfMaster;
class MediaPlayer;
#if (QT_VERSION_MAJOR >= 6)
class QAudioOutput;
#endif
class DrawTool;
class TextTool;
class PointingTool;
class SelectionTool;
class PathContainer;
class PixmapGraphicsItem;
class QPropertyAnimation;
class QXmlStreamReader;
class AbstractGraphicsPath;

namespace drawHistory {
    struct DrawToolDifference;
    struct TextPropertiesDifference;
    struct ZValueChange;
    struct Step;
}

namespace slide
{
    struct MediaItem
    {
        /// basic information about video from PDF
        MediaAnnotation annotation;
        /// QGraphicsItem representing the video
        QGraphicsVideoItem *item;
        /// Media player controling the video
        MediaPlayer *player;
#if (QT_VERSION_MAJOR >= 6)
        /// Audio output for media player.
        QAudioOutput *audio_out;
#endif
        /// Set of pages on which this video item appears. This is updated
        /// when videos for a new page are loaded and an old video is found
        /// to be visible also on the new page.
        std::set<int> pages;
    };
}
Q_DECLARE_METATYPE(slide::MediaItem);


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
    /// Container of objects required to handle a video and/or audio.
    /// @todo This implementation with different media players uses much memory.
    /// Settings for slide scenes, which apply to all views connected to the scene.
    enum SlideFlags
    {
        LoadMedia = 1 << 0,
        CacheVideos = 1 << 1,
        AutoplayVideo = 1 << 2,
        AutoplaySounds = 1 << 3,
        MuteSlide = 1 << 4,
        ShowTransitions = 1 << 5,
        ShowDrawings = 1 << 6,
        ShowSearchResults = 1 << 7,
        Default = 0xff,
    };

private:
    /// settings for this slide scene.
    quint8 slide_flags = SlideFlags::Default;

    /// Path which is currently being drawn.
    /// NULL if currenty no path is drawn.
    QGraphicsItem* currentlyDrawnItem {nullptr};

    /// Group of path segments forming the currently drawn path.
    /// This collection of segments is directly made visible and gets deleted
    /// when drawing the path is completed and the path itself is shown
    /// instead.
    QGraphicsItemGroup* currentItemCollection {nullptr};

    /// Searched results which should be highlighted
    /// This item gets many rectangles as child objects.
    QGraphicsItemGroup* searchResults {nullptr};

    /// Graphics item representing the PDF page background.
    PixmapGraphicsItem *pageItem {nullptr};
    /// Graphics item required during a page transition, usually
    /// represents the old page.
    PixmapGraphicsItem *pageTransitionItem {nullptr};

    /// List of (cached or active) video items.
    QList<slide::MediaItem> mediaItems;

    /// PDF document, including drawing paths.
    /// This is const, all data sent to master should be send via signals.
    const PdfMaster* master;

    /// Animation for slide transitions. Should be NULL while no
    /// slide transition is active.
    QAbstractAnimation *animation {NULL};

    /// Shift of currently shown page relative to the current presentation
    /// slide. Overlay specifications are stored in the nonzero bits of
    /// FirstOverlay and LastOverlay.
    int shift = 0;

    /// Currently visible page.
    int page = 0;

    /// Page part shown in this scene.
    const PagePart page_part;

    /// Bounding rect of all currently selected items.
    SelectionRectItem selection_bounding_rect;

    /// Start slide transition.
    void startTransition(const int newpage, const SlideTransition &transition);

    /// Search video annotation in cache and create + add it to cache if necessary.
    slide::MediaItem &getMediaItem(const MediaAnnotation &annotation, const int page);

    /// Create an animation object for a split slide transition.
    void createSplitTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a blinds slide transition.
    void createBlindsTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a box slide transition.
    void createBoxTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a wipe slide transition.
    void createWipeTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a fly slide transition.
    void createFlyTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem, PixmapGraphicsItem *oldPage);

    /// Create an animation object for a push slide transition.
    void createPushTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a cover slide transition.
    void createCoverTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for an uncover slide transition.
    void createUncoverTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

    /// Create an animation object for a face slide transition.
    void createFadeTransition(const SlideTransition &transition, PixmapGraphicsItem *pageTransitionItem);

public:
    /// Constructor: initialize master, page_part, and QGraphisScene.
    /// Connect signals.
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = NULL);

    /// Destructor: delete all graphics items.
    ~SlideScene();

    /// modifiable slide flags.
    quint8 &flags() noexcept
    {return slide_flags;}

    /// read-only slide flags
    const quint8 &flags() const noexcept
    {return slide_flags;}

    /// video items on all slides (cached or active).
    QList<slide::MediaItem> &getMedia() noexcept
    {return mediaItems;}

    /// Get current page item (the pixmap graphics item showing the current page)
    PixmapGraphicsItem *pageBackground() const noexcept
    {return pageItem;}

    /// Set shift in the form ((shift & ~AnyOverlay) | overlay).
    void setPageShift(const int relative_shift)
    {shift = relative_shift;}

    /// Get PdfMaster master.
    const PdfMaster* getPdfMaster()
    {return master;}

    /// Hash for this scene based on shift, master and page_part.
    size_t identifier() const
    {return qHash(QPair<int, const void*>(shift, master)) + page_part;}

    /// Currently visible page.
    int getPage() const
    {return page;}

    /// Shift in the form ((shift & ~AnyOverlay) | overlay).
    int getShift() const
    {return shift;}

    /// return page_part.
    PagePart pagePart() const
    {return page_part;}

    /// Handle tablet move event, mainly for drawing.
    /// Called from SlideView.
    void tabletMove(const QPointF &pos, const QTabletEvent *event)
    {
        handleEvents(
                    tablet_event_to_input_device(event) | Tool::UpdateEvent,
                    {pos},
                    QPointF(),
                    event->pressure()
                );
    }

    /// Handle tablet press event, mainly for drawing.
    /// Called from SlideView.
    void tabletPress(const QPointF &pos, const QTabletEvent *event)
    {
        handleEvents(
                    tablet_event_to_input_device(event) | Tool::StartEvent,
                    {pos},
                    QPointF(),
                    event->pressure()
                );
    }

    /// Handle tablet release event, mainly for drawing.
    /// Called from SlideView.
    void tabletRelease(const QPointF &pos, const QTabletEvent *event)
    {
        handleEvents(
                    tablet_event_to_input_device(event) | Tool::StopEvent,
                    {pos},
                    QPointF(),
                    event->pressure()
                );
    }

    /// Update geometry in preparation of navigation event.
    /// newpage does not include the shift.
    void prepareNavigationEvent(const int newpage);

    /// Receive navigation event from PdfMaster.
    /// The given page already includes the shift.
    void navigationEvent(const int newpage, SlideScene* newscene = NULL);

    /// Start handling draw and erase events.
    void startInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure = 1.);

    /// Handle draw and erase events.
    void stepInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure = 1.);

    /// Finish handling draw and erase events.
    bool stopInputEvent(const DrawTool *tool);

    /// Check if currently text is beeing edited.
    bool isTextEditing() const noexcept
    {return focusItem() && focusItem()->type() == TextGraphicsItem::Type;}

    /// Copy selection to clipboard.
    void copyToClipboard() const;

    /// Remove selection.
    void removeSelection() const;

    /// Paste from clipboard.
    void pasteFromClipboard();

protected:
    /**
     * @brief handle pointing device events.
     *
     * Handle mouse, touch and tablet input events for drawing,
     * highlighting and pointing.
     * Different types of pointing device events are unified and passed on to handleEvents.
     */
    bool event(QEvent *event) override;

    /// Clicked on slide without draw tool (follow link, play video, ...)
    bool noToolClicked(const QPointF &pos, const QPointF &startpos = QPointF());

    /// Handle events from different pointing devices.
    bool handleEvents(const int device, const QList<QPointF> &pos, const QPointF &start_pos, const float pressure);

    /// Helper function for handleEvents: draw tool events
    void handleDrawEvents(const DrawTool *tool, const int device, const QList<QPointF> &pos, const float pressure);
    /// Helper function for handleEvents: pointing tool events
    void handlePointingEvents(PointingTool *tool, const int device, const QList<QPointF> &pos);
    /// Helper function for handleEvents: selection tool events
    void handleSelectionEvents(SelectionTool *tool, const int device, const QList<QPointF> &pos, const QPointF &start_pos);
    /// Helper function for handleEvents: text tool events
    bool handleTextEvents(const TextTool *tool, const int device, const QList<QPointF> &pos);
    /// Handle selection start events (only called from handleEvents().
    void handleSelectionStartEvents(SelectionTool *tool, const QPointF &pos);
    /// Handle selection update events (only called from handleEvents().
    void handleSelectionUpdateEvents(SelectionTool *tool, const QPointF &pos, const QPointF &start_pos);
    /// Handle selection stop events (only called from handleEvents().
    void handleSelectionStopEvents(SelectionTool *tool, const QPointF &pos, const QPointF &start_pos);

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
    {invalidate(QRectF(), QGraphicsScene::ForegroundLayer);}

    /// Play all media on current slide.
    void playMedia() const;
    /// Pause all media on current slide.
    void pauseMedia() const;
    /// If anything is playing, pause everything. Otherwise play everything.
    void playPauseMedia() const;

    /// Update selection_bounding_rect.
    void updateSelectionRect() noexcept;

    /// Update tool, change selected items if necessary.
    void toolChanged(const Tool *tool) noexcept;
    /// Update draw tool color, change selected items if necessary.
    void colorChanged(const QColor &color) noexcept;
    /// Update draw tool width, change selected items if necessary.
    void widthChanged(const qreal width) noexcept;

    /// Show search results taken from PdfMaster.
    void updateSearchResults();

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    void navigationToViews(const int page, SlideScene* scene) const;

    /// Send a navigation signal (to master).
    void navigationSignal(const int page);

    /// Send action (to master).
    void sendAction(const Action action) const;

    /// Tell views to clear background.
    void clearViews() const;

    /// Send new path to PdfMaster.
    void sendNewPath(int page, QGraphicsItem *item) const;

    /// Send transformations for QGraphicsItems to PdfMaster.
    void sendHistoryStep(int page,
             std::map<QGraphicsItem*,QTransform> *transforms,
             std::map<AbstractGraphicsPath*,drawHistory::DrawToolDifference> *tools,
             std::map<TextGraphicsItem*,drawHistory::TextPropertiesDifference> *texts) const;

    /// Replace old path by new path in a single drawing history step.
    void replacePath(int page, QGraphicsItem *olditem, QGraphicsItem *newitem) const;

    /// Add new paths in single history step.
    void sendAddPaths(int page, const QList<QGraphicsItem*> &paths) const;

    /// Remove paths in single history step.
    void sendRemovePaths(int page, const QList<QGraphicsItem*> &paths) const;

    /// Tell master that transition has ended.
    void finishTransition();

    /// Get path container for given page, create one if cummulative drawing requires that.
    void requestNewPathContainer(PathContainer **container, int page);

    /// Get path container for given page, create one if it does not exist.
    void createPathContainer(PathContainer **container, int page);

    /// Notify master that there are unsaved changes.
    void newUnsavedDrawings();

    /// Bring given items to foreground and add history step.
    void bringToForeground(int page, const QList<QGraphicsItem*> &to_foreground) const;
    /// Bring given items to background and add history step.
    void bringToBackground(int page, const QList<QGraphicsItem*> &to_background) const;
};

/**
 * Read an SVG image. Create a GraphicsPictureItem which is added to target.
 * @param data read SVG image from this QByteArray
 * @param target add the GraphicsPictureItem constructed from the SVG image to this list
 */
void readFromSVG(const QByteArray &data, QList<QGraphicsItem*> &target);

/**
 * Read a pixel-based image. Create a GraphicsPictureItem which is added to target.
 * @param data read PNG image from this QByteArray
 * @param target add the GraphicsPictureItem constructed from the image to this list
 */
void readFromPixelImage(const QByteArray &data, QList<QGraphicsItem*> &target, const char *format);

/**
 * write items to an SVG image
 * @param data write result here
 * @param source take items from this list
 * @param rect sets viewBox of the SVG image
 */

void writeToSVG(QByteArray &data, const QList<QGraphicsItem*> &source, const QRectF &rect);
/**
 * write items to a pixel-based image
 * @param data write result here
 * @param source take items from this list
 * @param rect sets view box of the scene, which will be drawn to the image
 */
void writeToPixelImage(QByteArray &data, const QList<QGraphicsItem*> &source, const QRectF &rect, const qreal resolution = 1., const char *format = "PNG");

#endif // SLIDESCENE_H
