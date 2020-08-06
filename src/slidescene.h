#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTabletEvent>
#include "src/enumerates.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"

class PdfMaster;

/// QGraphicsScene for a presentation slide.
/// Handle drawing events and links?
class SlideScene : public QGraphicsScene
{
    Q_OBJECT

    /// Path which is currently being drawn
    AbstractGraphicsPath* currentPath {nullptr};
    QGraphicsItemGroup* currentItemCollection {nullptr};
    /// PDF document, including drawing paths.
    /// This is const, all data sent to master should be send via signals.
    const PdfMaster* master;
    /// Shift of currently shown page relative to the current presentation
    /// slide. Overlay specifications are stored in the nonzero bits of
    /// FirstOverlay and LastOverlay.
    int shift = 0;
    int page = 0;
    const PagePart page_part;

public:
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = nullptr);
    ~SlideScene();
    /// Set shift in the form (shift | overlay).
    void setPageShift(const int relative_shift) {shift = relative_shift;}
    const PdfMaster* getPdfMaster() {return master;}
    unsigned int identifier() const;
    int getPage() const {return page;}
    int getShift() const {return shift;}
    PagePart pagePart() const {return page_part;}
    void tabletMove(const QPointF &pos, const QTabletEvent *event);
    void tabletPress(const QPointF &pos, const QTabletEvent *event);
    void tabletRelease(const QPointF &pos, const QTabletEvent *event);
    void erase(const QPointF &pos);

protected:
    /// Handle various types of events
    virtual bool event(QEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();

public slots:
    void receiveAction(const Action action);
    /// Receive navigation event from PdfMaster.
    /// The given page already includes the shift.
    void navigationEvent(const int newpage, SlideScene* newscene = nullptr);

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    void navigationToViews(const int page, const QSizeF &size, SlideScene* scene) const;
    void sendNewPath(const int page, QGraphicsItem *item) const;
};

#endif // SLIDESCENE_H
