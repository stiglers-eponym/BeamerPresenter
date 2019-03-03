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

#ifndef PIDWIDCALLER_H
#define PIDWIDCALLER_H

#include <QWidget>
#include <QWindow>
#include <QProcess>
#include <QtDebug>

class PidWidCaller : public QProcess
{
    Q_OBJECT

public:
    explicit PidWidCaller(QString const& pid2wid, Q_PID const pid, int const page, int const index, QWidget* parent = nullptr);

private:
    int page;
    int index;

public slots:
    void sendResult(int const exitCode);

signals:
    void sendWid(WId const wid, int const page, int const index);
};

#endif // PIDWIDCALLER_H
