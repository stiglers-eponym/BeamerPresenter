/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "timer.h"

Timer::Timer(QWidget * parent) : QLabel(parent)
{
    setText("00:00");
    deadline = new QTime(0,0,0,0);
    time = new QTime(0,0,0,0);
}

Timer::Timer(QLineEdit * setTimerEdit, QWidget * parent) : QLabel(parent)
{
    setText("00:00");
    deadline = new QTime(0,0,0,0);
    time = new QTime(0,0,0,0);
    setTimerWidget(setTimerEdit);
}

Timer::~Timer()
{
    QObject::disconnect(timerEdit, &QLineEdit::returnPressed, this, &Timer::setDeadline);
    QObject::disconnect(timerEdit, &QLineEdit::returnPressed, this, &Timer::continueTimer);
    QObject::disconnect(timer, &QTimer::timeout, this, &Timer::showTime);
    delete deadline;
    delete time;
    delete timer;
}

void Timer::receiveTimerString(QString const & timerString)
{
    timerEdit->setText(timerString);
    setDeadline();
}

void Timer::setTimerWidget(QLineEdit * setTimerEdit)
{
    timerEdit = setTimerEdit;
    timer = new QTimer(this);
    QObject::connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::setDeadline);
    QObject::connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::continueTimer);
    QObject::connect(timer, &QTimer::timeout, this, &Timer::showTime);
    // TODO: connect escape in timer to sendEscape()
}

void Timer::setDeadline()
{
    switch ( timerEdit->text().length() )
    {
        case 1:
            *deadline = QTime::fromString(timerEdit->text(), "m");
        break;
        case 2:
            *deadline = QTime::fromString(timerEdit->text(), "mm");
        break;
        case 4:
            *deadline = QTime::fromString(timerEdit->text(), "m:ss");
        break;
        case 5:
            *deadline = QTime::fromString(timerEdit->text(), "mm:ss");
        break;
        case 7:
            *deadline = QTime::fromString(timerEdit->text(), "h:mm:ss");
        break;
    }
    if (time->secsTo(*deadline) > 0)
        emit sendNoAlert();
    else
        emit sendAlert();
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
}

void Timer::pauseTimer()
{
    if (running) {
        timer->stop();
        running = false;
    }
    else {
        timer->start(1000);
        running = true;
    }
}

void Timer::continueTimer()
{
    if (!deadline->isNull() && !running) {
        timer->start(1000);
        running = true;
    }
}

void Timer::resetTimer()
{
    time->setHMS(0,0,0);
    setText("00:00");
}

void Timer::showTime()
{
    *time = time->addSecs(1);
    if ( time->hour() )
        setText( time->toString("h:mm:ss") );
    else
        setText( time->toString("mm:ss") );
    int diff = time->secsTo(*deadline);
    if (diff == 0) {
        emit sendAlert();
    }
    if ((diff >=0) && (diff < colorTimeInterval)) {
        QPalette palette = QPalette();
        palette.setColor(QPalette::Window, QColor::fromRgb(256*diff/colorTimeInterval, 255, 256*diff/colorTimeInterval) );
        setPalette(palette);
    }
    else if ((diff < 0) && (diff > -colorTimeInterval)) {
        QPalette palette = QPalette();
        palette.setColor(QPalette::Window, QColor::fromRgb(-256*diff/colorTimeInterval, 255, 0) );
        setPalette(palette);
    }
    else if ((diff <= -colorTimeInterval) && (diff > -2*colorTimeInterval)) {
        QPalette palette = QPalette();
        palette.setColor(QPalette::Window, QColor::fromRgb(255, 511+256*diff/colorTimeInterval, 0) );
        setPalette(palette);
    }
}
