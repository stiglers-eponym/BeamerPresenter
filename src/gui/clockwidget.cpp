#include "src/gui/clockwidget.h"
#include "src/preferences.h"
#include <QTouchEvent>

ClockWidget::ClockWidget(QWidget *parent, bool accept_touch_input) :
    QLineEdit(parent),
    timer(new QTimer(this))
{
    setReadOnly(true);
    if (accept_touch_input)
        setAttribute(Qt::WA_AcceptTouchEvents);
    setAlignment(Qt::AlignCenter);
    setFocusPolicy(Qt::NoFocus);
    timer->setTimerType(Qt::CoarseTimer);
    QFont thefont = font();
    thefont.setPointSize(16);
    setFont(thefont);
    updateTime();
    connect(timer, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer->start(1000);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(20, 10);
    setToolTip("double-click on clock to start or pause timer");
}

void ClockWidget::resizeEvent(QResizeEvent *event) noexcept
{
    QFont thefont = font();
    thefont.setPointSizeF(std::min(event->size().height()*2/3, event->size().width()/6));
    setFont(thefont);
}

bool ClockWidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::TouchBegin:
        event->accept();
        return true;
    case QEvent::TouchEnd:
    {
        event->accept();
        QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
        if (tevent->pointCount() != 1)
            return false;
        debug_msg(DebugOtherInput) << "Touch event on clock widget:" << tevent;
        const QEventPoint &point = tevent->point(0);
        // Short touch with a single point should start/stop the timer.
        if (point.position() == point.pressPosition() && point.lastTimestamp() - point.pressTimestamp() < 100)
            emit sendAction(Action::StartStopTimer);
        return true;
    }
    default:
        return QLineEdit::event(event);
    }
}
