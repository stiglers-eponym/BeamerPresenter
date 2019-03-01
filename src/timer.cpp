/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "timer.h"

Timer::Timer(QWidget* parent) : QLabel(parent)
{
    setText("00:00");
}

Timer::Timer(QLineEdit* setTimerEdit, QWidget* parent) : QLabel(parent)
{
    setText("00:00");
    setTimerWidget(setTimerEdit);
    QPalette palette = QPalette(this->palette());
    palette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(palette);
}

Timer::~Timer()
{
    timerEdit->disconnect();
    timer->disconnect();
    delete timer;
}

void Timer::receiveTimerString(QString const& timerString)
{
    timerEdit->setText(timerString);
    setDeadline();
}

void Timer::setTimerWidget(QLineEdit* setTimerEdit)
{
    timerEdit = setTimerEdit;
    timer = new QTimer(this);
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::setDeadline);
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::sendEscape);
    connect(timer, &QTimer::timeout, this, &Timer::showTime);
    QPalette palette = QPalette(this->palette());
    palette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(palette);
    // TODO: connect escape in timer to sendEscape()
}

void Timer::setDeadline()
{
    switch (timerEdit->text().length())
    {
        case 1:
            deadline = QTime::fromString(timerEdit->text(), "m");
            break;
        case 2:
            deadline = QTime::fromString(timerEdit->text(), "mm");
            break;
        case 4:
            deadline = QTime::fromString(timerEdit->text(), "m:ss");
            break;
        case 5:
            deadline = QTime::fromString(timerEdit->text(), "mm:ss");
            break;
        case 7:
            deadline = QTime::fromString(timerEdit->text(), "h:mm:ss");
            break;
    }
    if (deadline.isNull() || !deadline.isValid()) {
        qCritical() << "Unable to set timer";
        deadline = QTime(0,0,0,0);
        setText("00:00");
    }
    QPalette palette = QPalette();
    int const diff = time.secsTo(deadline);
    if (diff > 0) {
        emit sendNoAlert();
        if (diff < colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(256*diff/colorTimeInterval, 255, 256*diff/colorTimeInterval) );
        else
            palette.setColor(QPalette::Window, Qt::white);
        setPalette(palette);
    }
    else {
        emit sendAlert();
        if (diff > -colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(-256*diff/colorTimeInterval, 255, 0) );
        else if (diff > -2*colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(255, 511+256*diff/colorTimeInterval, 0) );
        else
            palette.setColor(QPalette::Window, Qt::red);
    }
    if (!running)
        palette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(palette);
}

void Timer::pauseTimer()
{
    if (running) {
        timer->stop();
        running = false;
        QPalette palette = QPalette(this->palette());
        palette.setColor(QPalette::WindowText, Qt::gray);
        setPalette(palette);
    }
    else {
        timer->start(1000);
        running = true;
        QPalette palette = QPalette(this->palette());
        palette.setColor(QPalette::WindowText, Qt::black);
        setPalette(palette);
    }
}

void Timer::continueTimer()
{
    if (!deadline.isNull() && !running) {
        timer->start(1000);
        running = true;
        QPalette palette = QPalette(this->palette());
        palette.setColor(QPalette::WindowText, Qt::black);
        setPalette(palette);
    }
}

void Timer::resetTimer()
{
    time.setHMS(0,0,0);
    setText("00:00");
    QPalette palette = QPalette();
    int const diff = time.secsTo(deadline);
    if (diff > 0) {
        emit sendNoAlert();
        if (diff < colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(256*diff/colorTimeInterval, 255, 256*diff/colorTimeInterval) );
        else
            palette.setColor(QPalette::Window, Qt::white);
        setPalette(palette);
    }
    else {
        emit sendAlert();
        if (diff > -colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(-256*diff/colorTimeInterval, 255, 0) );
        else if (diff > -2*colorTimeInterval)
            palette.setColor(QPalette::Window, QColor::fromRgb(255, 511+256*diff/colorTimeInterval, 0) );
        else
            palette.setColor(QPalette::Window, Qt::red);
    }
    if (!running)
        palette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(palette);
}

void Timer::showTime()
{
    time = time.addSecs(1);
    if (time.hour()!=0)
        setText(time.toString("h:mm:ss"));
    else
        setText(time.toString("mm:ss"));
    int diff = time.secsTo(deadline);
    if (diff == 0) {
        emit sendAlert();
    }
    if ((diff >=0 ) && (diff < colorTimeInterval)) {
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

void Timer::receiveTimeoutInterval(int const interval)
{
    colorTimeInterval = interval;
}
