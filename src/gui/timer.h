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
#include <iostream>
#include <iomanip>
#include "../pdf/pdfdoc.h"

/// Maximum time between GUI updates in ms.
static const quint16 MAX_UPDATE_GUI_INTERVAL_MS = 1000;
/// Minimum time between GUI updates in ms.
static const quint16 MIN_UPDATE_GUI_INTERVAL_MS = 40;

class Timer : public QLabel
{
    Q_OBJECT

public:
    Timer(QWidget* parent = nullptr);
    ~Timer();
    /// Set times per slide for timer color change.
    void setTimeMap(QMap<QString, quint32> const& labelMap);
    void pauseTimer();
    void continueTimer();
    void toggleTimer();
    void resetTimer();
    void setLog(bool const set_log) {log = set_log;}
    /// Update update_gui_interval to the time scale at which the timer color changes.
    /// The argument is the minimum number of frames used for a transition between two colors.
    void updateGuiInterval(quint16 const frames);
    /// Set the timer time as a string.
    void setString(QString const& string);
    /// Set the timer colors.
    void setColors(QList<qint32> const& times, QList<QColor> const& colors);
    /// Set the QLineEdit widget and the presentation document.
    /// This function MUST be called before the timer can be used!
    void init(QLineEdit* setTimerEdit, PdfDoc const* presentation);
    QTimer* getTimer() {return timer;}

public slots:
    void setPage(int const pageNumber);

private slots:
    void setDeadline();
    void showTime();

signals:
    void sendAlert();
    void sendNoAlert();
    void sendEscape();

private:
    void updateColor();
    /// PDF document presentation
    PdfDoc const* doc = nullptr;
    /// Editable timer (GUI).
    QLineEdit* timerEdit = nullptr;
    /// Time at which the presentation is expected to end (in ms since epoche start).
    qint64 deadline;
    /// Current date and time minus time spent with the timer running (in ms since epoche start).
    /// This is a virtual starting time, corrected by time for which the timer was paused.
    qint64 startTime;
    /// Time at which the timer was paused (in ms since epoche start).
    qint64 pauseTime;
    /// QTimer triggering updates of GUI timer and of clock (ControlScreen::ui->clock_label).
    QTimer* timer;
    /// List of time differences (in ms) associated with colors.
    QList<qint32> colorTimes = {0};
    QList<QColor> colors = {Qt::white};
    QPalette timerPalette;
    /// Map slide numbers to time (in ms).
    QMap<int, quint32> timeMap;
    QMap<int, quint32>::const_iterator currentPageTimeIt;
    bool log = false;
    /// Time between GUI updates in ms.
    quint16 update_gui_interval = MAX_UPDATE_GUI_INTERVAL_MS;
};

#endif // TIMER_H
