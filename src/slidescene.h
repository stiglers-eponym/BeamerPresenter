#ifndef SLIDESCENE_H
#define SLIDESCENE_H

#include <QGraphicsScene>

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
};

#endif // SLIDESCENE_H
