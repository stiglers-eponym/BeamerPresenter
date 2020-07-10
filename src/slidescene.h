#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"

/// QGraphicsScene for a presentation slide.
/// Handle drawing events and links?
class SlideScene : public QGraphicsScene
{
    Q_OBJECT

private:
    /// Path which is currently being drawn
    QList<QGraphicsLineItem*> currentPath;

public:
    explicit SlideScene(QObject *parent = nullptr);

protected:
    /// Handle various types of events
    virtual bool event(QEvent *event) override;
    /// Stop drawing and convert just drawn path to regular path.
    void stopDrawing();
};

#endif // SLIDESCENE_H
