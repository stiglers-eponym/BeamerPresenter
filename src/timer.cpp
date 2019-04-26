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
    timerPalette = QPalette(this->palette());
    setText("00:00");
}

Timer::Timer(QLineEdit* setTimerEdit, QWidget* parent) : QLabel(parent)
{
    setText("00:00");
    setTimerWidget(setTimerEdit);
    timerPalette = QPalette(this->palette());
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(timerPalette);
}

Timer::~Timer()
{
    timerEdit->disconnect();
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
    connect(timerEdit, &QLineEdit::editingFinished, this, &Timer::setDeadline); // TODO: Check whether this causes problems.
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::setDeadline);
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::sendEscape);
    connect(timer, &QTimer::timeout, this, &Timer::showTime);
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(timerPalette);
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
        throw 1;
    }
    if (timeMap.isEmpty())
        currentFrameTime = &deadline;
    if (!running)
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
    if (deadline > time)
        emit sendNoAlert();
    else
        emit sendAlert();
    updateColor();
}

void Timer::pauseTimer()
{
    if (running) {
        timer->stop();
        running = false;
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
        setPalette(timerPalette);
    }
    else {
        timer->start(1000);
        running = true;
        timerPalette.setColor(QPalette::WindowText, Qt::black);
        setPalette(timerPalette);
    }
}

void Timer::continueTimer()
{
    if (!deadline.isNull() && !running) {
        timer->start(1000);
        running = true;
        timerPalette.setColor(QPalette::WindowText, Qt::black);
        setPalette(timerPalette);
    }
}

void Timer::resetTimer()
{
    time.setHMS(0,0,0);
    setText("00:00");
    if (!running)
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
    emit sendNoAlert();
    timerPalette.setColor(QPalette::Window, colors.first());
    setPalette(timerPalette);
    return;
}

void Timer::showTime()
{
    time = time.addSecs(1);
    if (time.hour()!=0)
        setText(time.toString("h:mm:ss"));
    else
        setText(time.toString("mm:ss"));
    if (time == deadline)
        emit sendAlert();
    updateColor();
}

void Timer::updateColor()
{
    int const diff = currentFrameTime->secsTo(time);
    if (diff <= colorTimes[0]) {
        timerPalette.setColor(QPalette::Window, colors[0]);
        setPalette(timerPalette);
        return;
    }
    for (int i=1; i<colorTimes.length(); i++) {
        if (diff <= colorTimes[i]) {
            double rel = double(diff - colorTimes[i-1])/(colorTimes[i]-colorTimes[i-1]);
            double irel = 1.-rel;
            timerPalette.setColor(QPalette::Window, QColor(int(rel*colors[i].red()+irel*colors[i-1].red()), int(rel*colors[i].green()+irel*colors[i-1].green()), int(rel*colors[i].blue()+irel*colors[i-1].blue()), int(rel*colors[i].alpha()+irel*colors[i-1].alpha())));
            setPalette(timerPalette);
            return;
        }
    }
    timerPalette.setColor(QPalette::Window, colors.last());
    setPalette(timerPalette);
}

void Timer::setTimeMap(QMap<int, QTime> &timeMap)
{
    this->timeMap = timeMap;
    if (timeMap.isEmpty())
        currentFrameTime = &deadline;
    else
        currentFrameTime = &timeMap.last();
}

void Timer::setPage(int const page)
{
    if (!timeMap.isEmpty())
        currentFrameTime = &*timeMap.upperBound(page);
    updateColor();
}
