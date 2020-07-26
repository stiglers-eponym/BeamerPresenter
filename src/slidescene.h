#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
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
    QList<QGraphicsLineItem*> currentPath;
    /// PDF document, including drawing paths.
    /// This is const, all data sent to master should be send via signals.
    const PdfMaster* master;
    /// Shift of currently shown page relative to the current presentation
    /// slide. Stored in the form (shift | overlay).
    int shift = 0;
    const PagePart page_part;

public:
    explicit SlideScene(const PdfMaster *master, const PagePart part = FullPage, QObject *parent = nullptr);
    /// Set shift in the form (shift | overlay).
    void setPageShift(const int relative_shift) {shift = relative_shift;}
    const PdfMaster* getPdfMaster() {return master;}
    unsigned int identifier() const;

protected:
    /// Handle various types of events
    virtual bool event(QEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();

public slots:
    void receiveAction(const Action action);
    /// Receive global navigation event.
    /// The given page does not include shift.
    void navigationEvent(const int page);

signals:
    /// Send navigation event to views.
    /// Here page is already adapted to shift.
    void navigationToViews(const int page, const QSizeF &size);
};

#endif // SLIDESCENE_H
