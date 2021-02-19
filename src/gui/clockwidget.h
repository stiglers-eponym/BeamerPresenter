#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QResizeEvent>

/**
 * @brief Label showing the time.
 */
class ClockWidget : public QLabel
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

private slots:
    void updateTime()
    {setText(QTime::currentTime().toString(Qt::TextDate));}
};

#endif // CLOCKWIDGET_H
