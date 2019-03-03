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

#include "pidwidcaller.h"

PidWidCaller::PidWidCaller(QString const& pid2wid, Q_PID const pid, int const page, int const index, QWidget* parent) : QProcess(parent)
{
    this->index = index;
    this->page = page;
    QStringList arguments;
    arguments << QString::number(pid);
    qDebug() << "Calling PID to WID:" << pid2wid << arguments;
    start(pid2wid, arguments);
    connect(this, SIGNAL(finished(int)), this, SLOT(sendResult(int)));
}

void PidWidCaller::sendResult(int const exitCode)
{
    if (exitCode != 0) {
        qCritical() << "Call to external translator from PID to Window ID failed, exit code" << exitCode;
        return;
    }
    char output[64];
    qint64 outputLength = readLine(output, sizeof(output));
    if (outputLength != -1) {
        QString winIdString(output);
        qDebug() << "Return value of PID to WID:" << winIdString;
        bool success;
        WId wid = (WId) winIdString.toLongLong(&success, 10);
        if (success && wid!=0)
            emit sendWid(wid, page, index);
        else
            qWarning() << "Could not read window ID";
    }
    else
        qCritical() << "Call to external translator from PID to Window ID had unexpected output";
    waitForFinished(10000);
    deleteLater();
}
