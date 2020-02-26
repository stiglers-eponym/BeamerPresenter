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
    setText("0");
#ifdef DISABLE_TOOL_TIP
#else
    setToolTip("Time since beginning of the presentation in (h:)mm:ss");
#endif
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
    connect(timerEdit, &QLineEdit::editingFinished, this, &Timer::setDeadline);
    connect(timerEdit, &QLineEdit::returnPressed, this, &Timer::sendEscape);
    connect(timer, &QTimer::timeout, this, &Timer::showTime);
    timer->start(update_gui_interval);
    timerPalette.setColor(QPalette::WindowText, Qt::gray);
#ifdef DISABLE_TOOL_TIP
#else
    timerEdit->setToolTip("Estimated time of the presentation in minutes or (h:)mm:ss");
#endif
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
    // Do not stop this->timer, because it is also used to update the clock (ControlScreen::ui->clock_label).
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
    }
}

void Timer::resetTimer()
{
    setText("0");
    deadline -= startTime;
    startTime = QDateTime::currentMSecsSinceEpoch();
    deadline += startTime;
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
    if (pauseTime == 0) {
        int const diff = QDateTime::currentMSecsSinceEpoch() - startTime;
        if (diff < 3600000)
            setText(QTime::fromMSecsSinceStartOfDay(diff).toString("m:ss"));
        else
            setText(QTime::fromMSecsSinceStartOfDay(diff).toString("h:mm:ss"));
        if (deadline <= startTime + diff && startTime + diff - deadline <= 2*update_gui_interval)
            emit sendAlert();
        updateColor();
    }
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

void Timer::setTimeMap(QMap<int, quint32> const& timeMap)
{
    this->timeMap = timeMap;
    currentPageTimeIt = timeMap.cbegin();
}

void Timer::setPage(int const pageLabel, int const pageNumber)
{
    currentPageTimeIt = timeMap.upperBound(pageLabel - 1);
    updateColor();
    if (log) {
        std::cout
                << std::setw(8) << QTime::currentTime().toString("h:mm:ss").toStdString()
                << std::setw(10) << QTime::fromMSecsSinceStartOfDay(QDateTime::currentMSecsSinceEpoch() - startTime).toString("h:mm:ss").toStdString()
                << "    entered page"
                << std::setw(4) << pageNumber + 1
                << std::setw(6) << "("+std::to_string(pageLabel)+").";
        if (currentPageTimeIt != timeMap.cend())
            std::cout
                    << "    Target time for page"
                    << std::setw(6) << "("+std::to_string(currentPageTimeIt.key())+")"
                    << " is"
                    << std::setw(9) << QTime::fromMSecsSinceStartOfDay(*currentPageTimeIt).toString("h:mm:ss").toStdString()
                    << ".";
        std::cout << std::endl;
    }
}

void Timer::updateGuiInterval(quint16 const frames)
{
    if (colorTimes.size() < 2)
        return;
    unsigned int min_delta = colorTimes[1] - colorTimes[0];
    unsigned int delta;
    for (int i=2; i<colorTimes.size(); i++) {
        delta = colorTimes[i] - colorTimes[i-1];
        if (delta < min_delta && delta > 0)
            min_delta = delta;
    }
    if (min_delta > frames * MAX_UPDATE_GUI_INTERVAL_MS)
        update_gui_interval = MAX_UPDATE_GUI_INTERVAL_MS;
    else if (min_delta <= frames * MIN_UPDATE_GUI_INTERVAL_MS)
        update_gui_interval = MIN_UPDATE_GUI_INTERVAL_MS;
    else {
        // Set update_gui_interval such that enough frames are shown for each color transition.
        update_gui_interval = min_delta / frames;
        // Decrease update_gui_interval such that 1000/update_gui_interval is (approximately) an integer.
        // Without this, the clock could be updated in irregular intervals.
        update_gui_interval = 1000. / (999/update_gui_interval + 1) + .999999999;
    }
    timer->setInterval(update_gui_interval);
    qDebug() << "Set update GUI inverval to" << update_gui_interval << ". Min delta was" << min_delta << "frames=" << frames;
}

void Timer::setString(QString const& string)
{
    timerEdit->setText(string);
    setDeadline();
}

void Timer::setColors(QList<qint32> const& times, QList<QColor> const& colors)
{
    this->colors = colors;
    colorTimes = times;
}
