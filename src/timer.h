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

class Timer : public QLabel
{
    Q_OBJECT

public:
    Timer(QWidget* parent = nullptr);
    Timer(QLineEdit* setTimerEdit, QWidget* parent = nullptr);
    ~Timer();
    void setTimerWidget(QLineEdit* setTimerEdit);
    void setTimeMap(QMap<int, QTime>& timeMap);

public slots:
    void setDeadline();
    void pauseTimer();
    void resetTimer();
    void continueTimer();
    void setPage(int const page);
    void receiveTimerString(QString const & timerString);
    void receiveColors(QList<int> times, QList<QColor> colors) {colorTimes=times; this->colors=colors;}

private slots:
    void showTime();

signals:
    void sendAlert();
    void sendNoAlert();
    void sendEscape();

private:
    void updateColor();
    QLineEdit* timerEdit;
    QTime deadline = QTime(0,0,0,0);
    QTime time = QTime(0,0,0,0);
    QTimer* timer;
    bool running = false;
    QList<int> colorTimes = {0};
    QList<QColor> colors = {Qt::white};
    QPalette timerPalette;
    QMap<int, QTime> timeMap = {};
    QTime const* currentFrameTime = &deadline;
};

#endif // TIMER_H
