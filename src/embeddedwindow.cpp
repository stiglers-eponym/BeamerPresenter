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

#include "embeddedwindow.h"

EmbeddedWindow::EmbeddedWindow(QProcess* process, QWidget* parent) : QWidget(parent)
{
    qDebug() << "Creating embedded window";
    qDebug() << "Program: " << process->program();
    this->process = process;
    // The program should output the window ID to standard output in hexadecimal format.
    char output[64];
    qDebug() << "Start reading standard output of process";
    qint64 outputLength = process->readLine(output, sizeof(output));
    qDebug() << "Finished reading standard output of process";
    if (outputLength == -1) {
        qWarning() << "Could not read window id";
        return;
    }
    QString winIdString(output);
    qDebug() << "Output was: " << winIdString;
    bool success;
    WId wid = (WId) winIdString.toLongLong(&success, 10);
    if (!success) {
        qWarning() << "Could not read window id";
        return;
    }
    qDebug() << "Window ID: " << wid;
    // TODO: get the window ID from process (PID) in X11

    window = QWindow::fromWinId(wid);
}

EmbeddedWindow::~EmbeddedWindow()
{
    process->kill();
    delete window;
    delete process;
}


void EmbeddedWindow::setProcess(QProcess* process)
{
    this->process = process;
}

void EmbeddedWindow::setWindow(QWindow* window)
{
    this->window = window;
}
