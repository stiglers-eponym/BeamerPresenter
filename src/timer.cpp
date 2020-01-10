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
    deadline(QDateTime::currentMSecsSinceEpoch()),
    startTime(deadline),
    pauseTime(deadline),
    timeMap(),
    currentPageTimeIt(timeMap.cend())
{
    timerPalette = QPalette(this->palette());
    setText("00:00");
}

Timer::Timer(QLineEdit* setTimerEdit, QWidget* parent) :
    QLabel(parent),
    deadline(QDateTime::currentMSecsSinceEpoch()),
    startTime(deadline),
    pauseTime(deadline),
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
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::sendEscape);
    connect(timer, &QTimer::timeout, this, &Timer::showTime);
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(timerPalette);
}

void Timer::setDeadline()
{
    bool ok;
    QStringList timeStringList = timerEdit->text().split(":");
    if (timeStringList.length() == 1)
        timeStringList = timerEdit->text().replace(".", ":").split(":");
    unsigned int time;
    switch (timeStringList.length())
    {
    case 1:
        // Expect value in minuts, convert to ms.
        time = 60000*timeStringList[0].toUInt(&ok);
        break;
    case 2:
        // Expect value in minuts, convert to ms.
         time = 60000*timeStringList[0].toUInt(&ok);
         if (ok)
            // Expect value in s, convert to ms.
             time += 1000*timeStringList[1].toDouble(&ok);
        break;
    case 3:
        // Expect value in h, convert to ms.
        time = 3600000*timeStringList[0].toUInt(&ok);
        if (ok)
            // Expect value in minuts, convert to ms.
            time += 60000*timeStringList[1].toUInt(&ok);
        if (ok)
            // Expect value in s, convert to ms.
            time += 1000*timeStringList[2].toDouble(&ok);
        break;
    default:
        ok = false;
        time = 0;
    }
    if (!ok) {
        qWarning() << "Did not understand time input" << timerEdit->text();
        timerEdit->setText("?");
        timerEdit->setFocus();
    }
    deadline = startTime + time;
    if (pauseTime != 0)
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
    if (QDateTime::currentMSecsSinceEpoch() >= deadline)
        emit sendAlert();
    else
        emit sendNoAlert();
    updateColor();
}

void Timer::toggleTimer()
{
    if (pauseTime == 0)
        pauseTimer();
    else
        continueTimer();
}

void Timer::pauseTimer()
{
    timer->stop();
    pauseTime = QDateTime::currentMSecsSinceEpoch();
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
    setPalette(timerPalette);
}

void Timer::continueTimer()
{
    if (pauseTime != 0) {
        qint64 const diff = QDateTime::currentMSecsSinceEpoch() - pauseTime;
        deadline += diff;
        startTime += diff;
        pauseTime = 0;
        timerPalette.setColor(QPalette::WindowText, Qt::black);
        setPalette(timerPalette);
        showTime();
        timer->start(UPDATE_GUI_INTERVAL_MS);
    }
}

void Timer::resetTimer()
{
    setText("00:00");
    startTime = QDateTime::currentMSecsSinceEpoch();
    if (pauseTime != 0) {
        pauseTime = startTime;
        timerPalette.setColor(QPalette::WindowText, Qt::gray);
    }
    emit sendNoAlert();
    timerPalette.setColor(QPalette::Window, colors.first());
    setPalette(timerPalette);
    return;
}

void Timer::showTime()
{
    int const diff = QDateTime::currentMSecsSinceEpoch() - startTime;
    if (diff < 3600000)
        setText(QTime::fromMSecsSinceStartOfDay(diff).toString("mm:ss"));
    else
        setText(QTime::fromMSecsSinceStartOfDay(diff).toString("h:mm:ss"));
    if (abs(deadline - startTime - diff) < UPDATE_GUI_INTERVAL_MS)
        emit sendAlert();
    updateColor();
}

void Timer::updateColor()
{
    int diff;
    if (currentPageTimeIt == timeMap.cend())
        diff = QDateTime::currentMSecsSinceEpoch() - deadline;
    else
        diff = QDateTime::currentMSecsSinceEpoch() - startTime - *currentPageTimeIt;
    if (diff <= colorTimes[0]) {
        timerPalette.setColor(QPalette::Window, colors[0]);
        setPalette(timerPalette);
        return;
    }
    for (int i=1; i<colorTimes.length(); i++) {
        if (diff <= colorTimes[i]) {
            double rel = double(diff - colorTimes[i-1])/(colorTimes[i] - colorTimes[i-1]);
            double irel = 1. - rel;
            timerPalette.setColor(QPalette::Window, QColor(int(rel*colors[i].red()+irel*colors[i-1].red()), int(rel*colors[i].green()+irel*colors[i-1].green()), int(rel*colors[i].blue()+irel*colors[i-1].blue()), int(rel*colors[i].alpha()+irel*colors[i-1].alpha())));
            setPalette(timerPalette);
            return;
        }
    }
    timerPalette.setColor(QPalette::Window, colors.last());
    setPalette(timerPalette);
}

void Timer::setTimeMap(QMap<int, quint32> &timeMap)
{
    this->timeMap = timeMap;
    currentPageTimeIt = timeMap.cbegin();
}

void Timer::setPage(int const page)
{
    currentPageTimeIt = timeMap.upperBound(page-1);
    updateColor();
    if (log) {
        if (currentPageTimeIt == timeMap.cend())
            qInfo()
                    << "At" << QTime::currentTime().toString("h:mm:ss")
                    << "/" << QTime::fromMSecsSinceStartOfDay(QDateTime::currentMSecsSinceEpoch() - startTime).toString("h:mm:ss")
                    << "entered page" << page;
        else
            qInfo()
                    << "At" << QTime::currentTime().toString("h:mm:ss")
                    << "/" << QTime::fromMSecsSinceStartOfDay(QDateTime::currentMSecsSinceEpoch() - startTime).toString("h:mm:ss")
                    << "entered page" << page
                    << ". Target time for page" << currentPageTimeIt.key()
                    << "is" << QTime::fromMSecsSinceStartOfDay(*currentPageTimeIt).toString("h:mm:ss");
    }
}
