#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTabletEvent>
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"

class PdfMaster;

/// QGraphicsScene for a presentation slide.
/// Handles drawing events and links. (But links are not really supported yet).
class SlideScene : public QGraphicsScene
{
    Q_OBJECT

    /// Path which is currently being drawn.
    /// nullptr if currenty no path is drawn.
    AbstractGraphicsPath* currentPath {nullptr};

    /// Group of path segments forming the currently drawn path.
    /// This collection of segments is directly made visible and gets deleted
    /// when drawing the path is completed and the path itself is shown
    /// instead.
    QGraphicsItemGroup* currentItemCollection {nullptr};

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
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = nullptr);

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

    /// Receive navigation event from PdfMaster.
    /// The given page already includes the shift.
    void navigationEvent(const int newpage, SlideScene* newscene = nullptr);

protected:
    /// Handle various types of events.
    virtual bool event(QEvent *event) override;

    /// Handle mouse release event. Currently does nothing.
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();

public slots:
    /// Receive action. Currently does nothing.
    void receiveAction(const Action action);

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    /// size is given in points (inch/72).
    void navigationToViews(const int page, const QSizeF &size, SlideScene* scene) const;

    /// Send new path to PdfMaster.
    void sendNewPath(const int page, QGraphicsItem *item) const;
};

#endif // SLIDESCENE_H
