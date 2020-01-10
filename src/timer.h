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

#ifndef TIMER_H
#define TIMER_H

#include <QtDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTime>
#include <QTimer>

/// Time between GUI updates in ms.
static const unsigned short UPDATE_GUI_INTERVAL_MS = 1000;

class Timer : public QLabel
{
    Q_OBJECT
    friend class ControlScreen;

public:
    Timer(QWidget* parent = nullptr);
    Timer(QLineEdit* setTimerEdit, QWidget* parent = nullptr);
    ~Timer();
    void setTimeMap(QMap<int, quint32>& timeMap);
    void pauseTimer();
    void continueTimer();
    void toggleTimer();
    void resetTimer();
    void setLog(bool const set_log) {log = set_log;}

public slots:
    void setPage(int const page);

private slots:
    void setDeadline();
    void showTime();

signals:
    void sendAlert();
    void sendNoAlert();
    void sendEscape();

private:
    void setTimerWidget(QLineEdit* setTimerEdit);
    void updateColor();
    QLineEdit* timerEdit;
    /// Time at which the presentation is expected to end (in ms since epoche start).
    qint64 deadline;
    /// Current date and time minus time spent with the timer running (in ms since epoche start).
    /// This is a virtual starting time, corrected by time for which the timer was paused.
    qint64 startTime;
    /// Time at which the timer was paused (in ms since epoche start).
    qint64 pauseTime;
    /// QTimer triggering updates of GUI timer.
    QTimer* timer;
    /// List of time differences (in ms) associated with colors.
    QList<qint32> colorTimes = {0};
    QList<QColor> colors = {Qt::white};
    QPalette timerPalette;
    /// Map slide label (as integer) to time (in ms).
    QMap<int, quint32> timeMap;
    QMap<int, quint32>::const_iterator currentPageTimeIt;
    bool log = false;
};

#endif // TIMER_H
