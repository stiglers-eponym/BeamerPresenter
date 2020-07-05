#ifndef SLIDE_H
#define SLIDE_H

#include <QGraphicsView>

/// Slide shown on the screen: a view of SlideScene.
/// This also draws the background (PDF page) of the slide.
class SlideView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit SlideView(QWidget *parent = nullptr);

protected:
    /// Draw the slide to background (with correct resolution and from cache).
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

signals:

};

#endif // SLIDE_H
