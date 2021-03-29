#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QLineEdit>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QResizeEvent>
#include "src/enumerates.h"

/**
 * @brief Label showing the time.
 *
 * This is implemented as a QLineEdit because that one does not
 * request to update the layout when it's text is changed. There
 * is no need to recalculate the full layout every second because
 * the clock changes it's text.
 */
class ClockWidget : public QLineEdit
{
    Q_OBJECT

    /// Timer: regularly update clock.
    QTimer *timer;

public:
    /// Constructor
    explicit ClockWidget(QWidget *parent = NULL);

    /// Destructor: delete timer.
    ~ClockWidget()
    {delete timer;}

    /// Size hint: based on estimated size.
    QSize sizeHint() const noexcept override
    {return {125,20};}

    /// Height depends on width through font size.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override
    {setFont({"", std::min(event->size().height()*2/3, event->size().width()/6), 2});}

    // Disable mouse interaction.
    /// Disable mouse press events.
    void mousePressEvent(QMouseEvent*) noexcept override {}
    /// Disable mouse double click events.
    void mouseDoubleClickEvent(QMouseEvent*) noexcept override
    {emit sendAction(StartStopTimer);}
    /// Disable mouse move events.
    void mouseMoveEvent(QMouseEvent*) noexcept override {}
    /// Disable mouse Release events.
    void mouseReleaseEvent(QMouseEvent*) noexcept override {}

private slots:
    /// Update label to show current time.
    void updateTime()
    {setText(QTime::currentTime().toString(Qt::TextDate));}

signals:
    void sendAction(const Action);
};

#endif // CLOCKWIDGET_H
