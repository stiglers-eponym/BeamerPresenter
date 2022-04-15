#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <set>
#include <QAudioOutput>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QGraphicsSceneMouseEvent>
#include <QTabletEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"
#include "src/rendering/mediaplayer.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/drawing/selectionrectitem.h"

class PdfMaster;
class AbstractGraphicsPath;
class Tool;
class DrawTool;
class PathContainer;
class PixmapGraphicsItem;

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
    struct MediaItem
    {
        /// basic information about video from PDF
        PdfDocument::MediaAnnotation annotation;
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
        Default = 0xff,
    };

private:
    /// settings for this slide scene.
    quint8 slide_flags = SlideFlags::Default;

    /// Path which is currently being drawn.
    /// NULL if currenty no path is drawn.
    QGraphicsItem* currentlyDrawnItem {NULL};

    /// Group of path segments forming the currently drawn path.
    /// This collection of segments is directly made visible and gets deleted
    /// when drawing the path is completed and the path itself is shown
    /// instead.
    QGraphicsItemGroup* currentItemCollection {NULL};

    /// Graphics item representing the PDF page background.
    PixmapGraphicsItem *pageItem {NULL};
    /// Graphics item required during a page transition, usually
    /// represents the old page.
    PixmapGraphicsItem *pageTransitionItem {NULL};

    /// List of (cached or active) video items.
    QList<MediaItem> mediaItems;

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
    void startTransition(const int newpage, const PdfDocument::SlideTransition &transition);

    /// Search video annotation in cache and create + add it to cache if necessary.
    MediaItem &getMediaItem(const PdfDocument::MediaAnnotation &annotation, const int page);

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
    QList<MediaItem> &getMedia() noexcept
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
    unsigned int identifier() const
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
    void tabletMove(const QPointF &pos, const QTabletEvent *event);

    /// Handle tablet press event, mainly for drawing.
    /// Called from SlideView.
    void tabletPress(const QPointF &pos, const QTabletEvent *event);

    /// Handle tablet release event, mainly for drawing.
    /// Called from SlideView.
    void tabletRelease(const QPointF &pos, const QTabletEvent *event);

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
    void noToolClicked(const QPointF &pos, const QPointF &startpos = QPointF());

    /// Handle events from different pointing devices.
    void handleEvents(const int device, const QList<QPointF> &pos, const QPointF &start_pos, const float pressure);

protected slots:
    /// Update selection_bounding_rect.
    void updateSelectionRect() noexcept;

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

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    void navigationToViews(const int page, SlideScene* scene) const;

    /// Send a navigation signal (to master).
    void navigationSignal(const int page) const;

    /// Send action (to master).
    void sendAction(const Action action) const;

    /// Tell views to clear background.
    void clearViews() const;

    /// Send new path to PdfMaster.
    void sendNewPath(int page, QGraphicsItem *item) const;

    /// Send transformations for QGraphicsItems to PdfMaster.
    void sendTransformsCommon(int page, const QList<QGraphicsItem*> &item, const QTransform &transform) const;

    /// Replace old path by new path in a single drawing history step.
    void replacePath(int page, QGraphicsItem *olditem, QGraphicsItem *newitem) const;

    /// Tell master that transition has ended.
    void finishTransition();

    /// Get path container for given page, create one if necessary.
    void requestNewPathContainer(PathContainer **container, int page);

    /// Notify master that there are unsaved changes.
    void newUnsavedDrawings();
};

Q_DECLARE_METATYPE(SlideScene::MediaItem);

#endif // SLIDESCENE_H
