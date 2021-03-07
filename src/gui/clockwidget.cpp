#include "src/gui/clockwidget.h"

ClockWidget::ClockWidget(QWidget *parent) :
    QLabel(parent),
    timer(new QTimer(this))
{
    setAlignment(Qt::AlignCenter);
    setFocusPolicy(Qt::NoFocus);
    timer->setTimerType(Qt::CoarseTimer);
    setFont({"", 16, 2});
    updateTime();
    connect(timer, &QTimer::timeout, this, &ClockWidget::updateTime);
    timer->start(1000);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(20, 10);
}
