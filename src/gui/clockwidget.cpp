#include "src/gui/clockwidget.h"

ClockWidget::ClockWidget(QWidget *parent) :
    QLineEdit(parent),
    timer(new QTimer(this))
{
    setAlignment(Qt::AlignCenter);
    setFocusPolicy(Qt::NoFocus);
    setCursor(QCursor());
    timer->setTimerType(Qt::CoarseTimer);
    setFont({"", 16, 2});
    updateTime();
    connect(timer, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer->start(1000);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(20, 10);
    setToolTip("double-click on clock to start or pause timer");
}
