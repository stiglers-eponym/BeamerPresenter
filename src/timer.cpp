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

Timer::Timer(QWidget* parent) :
    QLabel(parent),
    deadline(QDateTime::currentDateTimeUtc()),
    startTime(QDateTime::currentDateTimeUtc()),
    timeMap(),
    currentPageTimeIt(timeMap.cend())
{
    timerPalette = QPalette(this->palette());
    setText("00:00");
}

Timer::Timer(QLineEdit* setTimerEdit, QWidget* parent) :
    QLabel(parent),
    deadline(QDateTime::currentDateTimeUtc()),
    startTime(QDateTime::currentDateTimeUtc()),
    timeMap(),
    currentPageTimeIt(timeMap.cend())
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
    QStringList timerText = timerEdit->text().replace(".", ":").split(":");
    qint64 diff;
    bool ok;
    switch (timerText.length())
    {
    case 1:
        diff = 60*timerText[0].toLong(&ok);
        break;
    case 2:
        diff = 60*timerText[0].toLong(&ok);
        if (ok)
             diff += timerText[1].toLong(&ok);
        break;
    case 3:
        diff = 3600*timerText[0].toLong(&ok);
        if (ok)
            diff += 60*timerText[1].toLong(&ok);
        if (ok)
            diff += timerText[2].toLong(&ok);
        break;
    default:
        diff = 0;
        ok = false;
    }
    if (!ok) {
        qWarning() << "Did not understand time input" << timerEdit->text();
        timerEdit->setText("?");
        timerEdit->setFocus();
    }
    deadline = startTime.addSecs(diff);
    if (!running)
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
    if (deadline > startTime)
        emit sendNoAlert();
    else
        emit sendAlert();
    updateColor();
}

void Timer::toggleTimer()
{
    if (running)
        pauseTimer();
    else
        continueTimer();
}

void Timer::pauseTimer()
{
    timer->stop();
    running = false;
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(timerPalette);
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
    startTime = QDateTime::currentDateTimeUtc();
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
    qint64 const diff = startTime.msecsTo(QDateTime::currentDateTimeUtc());
    if (diff < 3600000)
        setText(QTime::fromMSecsSinceStartOfDay(diff).toString("mm:ss"));
    else
        setText(QTime::fromMSecsSinceStartOfDay(diff).toString("h:mm:ss"));
    if (abs(QDateTime::currentDateTimeUtc().msecsTo(deadline)) < 1000)
        emit sendAlert();
    updateColor();
}

void Timer::updateColor()
{
    int diff;
    if (currentPageTimeIt == timeMap.cend())
        diff = deadline.secsTo(QDateTime::currentDateTimeUtc());
    else
        diff = startTime.secsTo(QDateTime::currentDateTimeUtc()) - *currentPageTimeIt;
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

void Timer::setTimeMap(QMap<int, qint64> &timeMap)
{
    this->timeMap = timeMap;
    currentPageTimeIt = timeMap.cbegin();
}

void Timer::setPage(int const page)
{
    currentPageTimeIt = timeMap.upperBound(page-1);
    updateColor();
}
