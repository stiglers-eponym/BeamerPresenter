#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTabletEvent>
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/flexgraphicslineitem.h"
#include "src/drawing/pointingtool.h"

class PdfMaster;

/**
 * @brief SlideScene: QGraphicsScene for a presentation slide.
 *
 * Handles drawing events and links. (But links are not really supported yet).
 */
class SlideScene : public QGraphicsScene
{
    Q_OBJECT

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

    /// Show slide transitions, multimedia, etc. (all not implemented yet).
    bool show_animations = true;

    /// Start slide transition. Experimental!
    void startTransition(const int newpage, const SlideTransition &transition);

public:
    /// Constructor: initialize master, page_part, and QGraphisScene.
    /// Connect signals.
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = NULL);

    /// Destructor: delete all graphics items.
    ~SlideScene();

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
    bool stopInputEvent(const QPointF &pos);

protected:
    /**
     * @brief handle pointing device events.
     * @param event
     *
     * Handle mouse, touch and tablet input events for drawing,
     * highlighting and pointing.
     */
    bool event(QEvent *event) override;

    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();

public slots:
    /// Receive action. Currently does nothing.
    void receiveAction(const Action action);

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    /// size is given in points (inch/72).
    void navigationToViews(const int page, SlideScene* scene) const;

    /// Tell views to clear background.
    void clearViews() const;

    /// Send new path to PdfMaster.
    void sendNewPath(const int page, QGraphicsItem *item) const;

    void showMagnifier(QPainter *painter, const PointingTool *tool);
};

#endif // SLIDESCENE_H
