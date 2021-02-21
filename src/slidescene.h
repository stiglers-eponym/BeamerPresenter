#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsVideoItem>
#include <QGraphicsSceneMouseEvent>
#include <QTabletEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPropertyAnimation>
#include <QTimer>
#include "src/enumerates.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/flexgraphicslineitem.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/pathcontainer.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/pixmapgraphicsitem.h"

class PdfMaster;

/**
 * @brief SlideScene: QGraphicsScene for a presentation slide.
 *
 * Handles drawing events and links. (But links are not really supported yet).
 */
class SlideScene : public QGraphicsScene
{
    Q_OBJECT

public:
    struct VideoItem
    {
        MediaAnnotation annotation;
        QGraphicsVideoItem *item;
        QMediaPlayer *player;
        QSet<int> pages;
    };

private:
    /// Path which is currently being drawn.
    /// NULL if currenty no path is drawn.
    AbstractGraphicsPath* currentPath {NULL};

    /// Currently used draw tool, cached during drawing.
    /// current_tool is never owned by this.
    Tool *current_tool {NULL};

    /// Group of path segments forming the currently drawn path.
    /// This collection of segments is directly made visible and gets deleted
    /// when drawing the path is completed and the path itself is shown
    /// instead.
    QGraphicsItemGroup* currentItemCollection {NULL};

    /// Graphics item representing the PDF page background.
    PixmapGraphicsItem *pageItem {NULL};
    PixmapGraphicsItem *pageTransitionItem {NULL};

    /// List of (cached or active) video items.
    QList<VideoItem> videoItems;

    /// PDF document, including drawing paths.
    /// This is const, all data sent to master should be send via signals.
    const PdfMaster* master;

    /// Shift of currently shown page relative to the current presentation
    /// slide. Overlay specifications are stored in the nonzero bits of
    /// FirstOverlay and LastOverlay.
    int shift = 0;

    /// Currently visible page.
    int page = 0;

    /// Page part shown in this scene.
    const PagePart page_part;

    QPropertyAnimation *animation {NULL};

    /// Start slide transition.
    void startTransition(const int newpage, const SlideTransition &transition);

    /// Search video annotation in cache and create + add it to cache if necessary.
    VideoItem &getVideoItem(const MediaAnnotation &annotatio, const int pagen);

public:
    /// Constructor: initialize master, page_part, and QGraphisScene.
    /// Connect signals.
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = NULL);

    /// Destructor: delete all graphics items.
    ~SlideScene();

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
    void startInputEvent(Tool *tool, const QPointF &pos, const float pressure = 1.);

    /// Handle draw and erase events.
    void stepInputEvent(const QPointF &pos, const float pressure = 1.);

    /// Finish handling draw and erase events.
    bool stopInputEvent(const QPointF &pos = {0,0});

    /// Check if currently a draw tool is active, i.e. something is drawn.
    bool isDrawing() const noexcept
    {return current_tool;}

    /// Check if currently text is beeing edited.
    bool isTextEditing() const noexcept
    {return focusItem() && focusItem()->type() == QGraphicsTextItem::Type;}

protected:
    /**
     * @brief handle pointing device events.
     * @param event
     *
     * Handle mouse, touch and tablet input events for drawing,
     * highlighting and pointing.
     */
    bool event(QEvent *event) override;

    /// Clicked on slide without draw tool (follow link, play video, ...)
    void noToolClicked(const QPointF &pos, const QPointF &startpos = QPointF());

public slots:
    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();

    /// Receive action. Currently does nothing.
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
    /// size is given in points (inch/72).
    void navigationToViews(const int page, SlideScene* scene) const;

    /// Tell views to clear background.
    void clearViews() const;

    /// Send new path to PdfMaster.
    void sendNewPath(int page, QGraphicsItem *item) const;

    void beginTransition(const SlideTransition &transition, PixmapGraphicsItem *transitionItem);
    void finishTransition();

    void requestPathContainer(PathContainer **container, int page);
};

Q_DECLARE_METATYPE(SlideScene::VideoItem);

#endif // SLIDESCENE_H
