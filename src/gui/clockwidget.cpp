// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <algorithm>
#include <QtConfig>
#include <QTime>
#include <QFont>
#include <QTouchEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#if (QT_VERSION_MAJOR >= 6)
#include <QEventPoint>
#endif
#include "src/log.h"
#include "src/gui/clockwidget.h"

ClockWidget::ClockWidget(bool accept_touch_input, QWidget *parent) :
    QLineEdit(parent)
{
    setReadOnly(true);
    if (accept_touch_input)
        setAttribute(Qt::WA_AcceptTouchEvents);
    setAlignment(Qt::AlignCenter);
    setFocusPolicy(Qt::NoFocus);
    QFont thefont = font();
    thefont.setPointSize(16);
    setFont(thefont);
    setText(QTime::currentTime().toString(Qt::TextDate));
    startTimer(1000);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(20, 10);
    setToolTip(tr("double-click on clock to start or pause timer"));
}

void ClockWidget::resizeEvent(QResizeEvent *event) noexcept
{
    QFont thefont = font();
    thefont.setPointSizeF(std::min(event->size().height()*2/3, event->size().width()/6));
    setFont(thefont);
}

void ClockWidget::mouseDoubleClickEvent(QMouseEvent *event) noexcept
{
    emit sendAction(StartStopTimer);
    event->accept();
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
#if (QT_VERSION_MAJOR >= 6)
        if (tevent->pointCount() != 1)
            return false;
        debug_msg(DebugOtherInput, "Touch event on clock widget:" << tevent);
        const QEventPoint &point = tevent->point(0);
        // Short touch with a single point should start/stop the timer.
        if (point.position() == point.pressPosition() && point.lastTimestamp() - point.pressTimestamp() < 100)
            emit sendAction(Action::StartStopTimer);
#else
        if (tevent->touchPoints().length() != 1)
            return false;
        debug_msg(DebugOtherInput, "Touch event on clock widget:" << tevent);
        const QTouchEvent::TouchPoint &point = tevent->touchPoints().first();
        // Touch event with a single point should start/stop the timer.
        if (point.pos() == point.startPos())
            emit sendAction(Action::StartStopTimer);
#endif
        return true;
    }
    default:
        return QLineEdit::event(event);
    }
}
