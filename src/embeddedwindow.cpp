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

EmbeddedWindow::EmbeddedWindow(QWidget* parent) : QWidget(parent)
{
}

EmbeddedWindow* EmbeddedWindow::createWindow(QString const program, QStringList const arguments, QWidget* parent)
{
    QProcess* process = new QProcess(parent);
    process->start(program, arguments);

    // The program should output the window ID to standard output in hexadecimal format.
    char output[64];
    qint64 outputLength = process->readLine(output, sizeof(output));
    if (outputLength == -1) {
        qWarning() << "Could not read window id";
        return nullptr;
    }
    QString winIdString(output);
    bool success;
    WId wid = (WId) winIdString.toLongLong(&success, 16);
    if (!success) {
        qWarning() << "Could not read window id";
        return nullptr;
    }
    // TODO: get the window ID from process (PID) in X11

    QWindow* window = QWindow::fromWinId(wid);
    EmbeddedWindow* embedded = (EmbeddedWindow*) createWindowContainer(window, parent);
    embedded->setProcess(process);
    embedded->setWindow(window);
    return embedded;
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
