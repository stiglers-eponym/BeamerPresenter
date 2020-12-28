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

#include "embedapp.h"

EmbedApp::EmbedApp(QStringList const& command, QString const& pid2wid, int const page, int const index, QWidget* parent) :
    QObject(parent),
    pid2widTimer(new QTimer(this)),
    command(command),
    pid2wid(pid2wid)
{
    connect(pid2widTimer, &QTimer::timeout, this, &EmbedApp::getWidFromPid);
    pages.append(page);
    indices.append(index);
}

EmbedApp::~EmbedApp()
{
    // Everything gets deleted in EmbedApp::clearProcess when the process finishes.
    if (process!=nullptr && process->state()!=QProcess::NotRunning) {
        process->terminate();
        if (process->state()!=QProcess::NotRunning && !process->waitForFinished(10000)) {
            qCritical() << "Embedded process did not stop correctly. Killing it.";
            process->kill();
        }
    }
}

void EmbedApp::start()
{
    /* Start the embedded application.
     * The procedure for creating an embedded application is the following:
     *
     * 1. Create an EmbedApp object.
     * 2. Start the application with EmbedApp::start()
     *    Now it depends on whether pid2wid is empty:
     * 3a.  If pid2wid is empty:
     * 3a.1 Wait until the application writes to standard output.
     *      This will call createFromStdOut, which reads the window ID.
     *      If createFromStdOut succeeds, it will call create(wid).
     * 3b.  If pid2wid is not empty
     * 3b.1 pid2widTimer is started. On timeout it calls getWidFromPid
     * 3b.2 getWidFromPid stops pid2widTimer and creates a QProcess pid2widProcess,
     *      which tries to get the window ID from an external application.
     * 3b.3 When pid2widProcess writes to standard output, receiveWidFromPid tries to read the window ID.
     *      If receiveWidFromPid succeeds, it will call create(wid).
     *      If it fails, it restarts pid2widTimer with an increased interval.
     *      Thus the steps in 3b are repeated until receiveWidFromPid succeeds.
     * 4. create(wid) takes control over the external window and puts it in widget.
     * 5. emit widgetReady(this). PageWidget will then set the correct geometry and show the widget.
     */
    if (process != nullptr)
        return;
    if (pid2wid.isEmpty()) {
        // If there is no program which tells us the window ID from the progess ID, we hope that the application, which we want to embed, tells us its WID via standard output.
        process = new QProcess(this);
        connect(process, &QProcess::readyReadStandardOutput, this, &EmbedApp::createFromStdOut);
        connect(process, static_cast<void (QProcess::*)(int const, QProcess::ExitStatus const)>(&QProcess::finished), this, &EmbedApp::clearProcess);
#if QT_VERSION_MAJOR <= 5 and QT_VERSION_MINOR < 15
        process->start(command.join(" "));
#else
        if (command.length() == 1) {
            QStringList arguments = QProcess::splitCommand(command.first());
            process->start(arguments.takeFirst(), arguments);
        }
        else {
            QStringList arguments = command;
            process->start(arguments.takeFirst(), arguments);
        }
#endif
#ifdef DEBUG_MULTIMEDIA
        qDebug() << "Started process:" << process->program();
#endif
    }
    else {
        // If we know a program for converting process IDs to window IDs, this will be used to get the WID.
        process = new QProcess(this);
        connect(process, static_cast<void (QProcess::*)(int const, QProcess::ExitStatus const)>(&QProcess::finished), this, &EmbedApp::clearProcess);
#if QT_VERSION_MAJOR <= 5 and QT_VERSION_MINOR < 15
        process->start(command.join(" "));
#else
        if (command.length() == 1) {
            QStringList arguments = QProcess::splitCommand(command.first());
            process->start(arguments.takeFirst(), arguments);
        }
        else {
            QStringList arguments = command;
            process->start(arguments.takeFirst(), arguments);
        }
#endif
#ifdef DEBUG_MULTIMEDIA
        qDebug() << "Started process:" << process->program() << process->pid();
#endif
        // Wait some time before trying to get the window ID
        // The window has to be created first.
        pid2widTimer->start(minDelayPidWidCaller);
        // getWidFromPID will be called frequently until there exists a window corresponding to the process ID
        // The time between two calls of createEmbeddedWindowsFromPID will be increased exponentially.
    }
}

void EmbedApp::update()
{
    if (widget != nullptr && widget->isVisible())
        widget->update();
}

void EmbedApp::clearProcess(int const exitCode, QProcess::ExitStatus const exitStatus)
{
#ifdef DEBUG_MULTIMEDIA
    qDebug() << "Process ended";
#endif
    // Reset this to the state immediately after it was created.
    if (exitStatus == QProcess::CrashExit)
        qWarning() << "Embedded application crashed";
    if (exitCode !=0)
        qWarning() << "Embedded application finished with exit code" << exitCode;
    pid2widTimer->stop();
    if (pid2widProcess!=nullptr && pid2widProcess->state()!=QProcess::NotRunning) {
        pid2widProcess->terminate();
        pid2widProcess->waitForFinished(1000);
    }
    delete pid2widProcess;
    pid2widProcess = nullptr;
    delete widget;
    widget = nullptr;
    window = nullptr;
    if (process != nullptr && process->state() != QProcess::NotRunning) {
        process->terminate();
        if (process->state()!=QProcess::NotRunning && !process->waitForFinished(10000)) {
            qCritical() << "Embedded process did not stop correctly. Killing it.";
            process->kill();
        }
    }
    delete process;
    process = nullptr;
}

void EmbedApp::getWidFromPid()
{
    pid2widTimer->stop();
    if (pid2widProcess == nullptr) {
        pid2widProcess = new QProcess(this);
        connect(pid2widProcess, static_cast<void (QProcess::*)(int const, QProcess::ExitStatus const)>(&QProcess::finished), this, &EmbedApp::receiveWidFromPid);
    }
#ifdef Q_OS_WIN
    pid2widProcess->start(pid2wid, QStringList() << QString::number(process->pid()->dwProcessId));
#else
    pid2widProcess->start(pid2wid, QStringList() << QString::number(process->processId()));
#endif
}

void EmbedApp::receiveWidFromPid(int const exitCode, QProcess::ExitStatus const status)
{
    if (status == QProcess::CrashExit || exitCode != 0)
        qWarning() << "Call to external translator from PID to Window ID failed, exit code" << exitCode;
    if (process->state() == QProcess::NotRunning)
        qWarning() << "Process" << command.join(" ") << "not running. Assuming that it has ended.";
    else {
        char output[64];
        qint64 outputLength = pid2widProcess->readLine(output, sizeof(output));
        if (outputLength == -1) {
            qWarning() << "Call to external translator from PID to Window ID had unexpected output" << output;
            pid2widTimer->start(int(1.5*pid2widTimer->interval()));
        }
        else {
            QString winIdString(output);
#ifdef DEBUG_MULTIMEDIA
            qDebug() << "Return value of PID to WID:" << winIdString;
#endif
            bool success;
            WId wid = WId(winIdString.toLongLong(&success, 10));
            if (success && wid!=0)
                create(wid);
            else
                pid2widTimer->start(int(1.5*pid2widTimer->interval()));
        }
    }
    pid2widProcess->waitForFinished(1000);
    delete pid2widProcess;
    pid2widProcess = nullptr;
}

void EmbedApp::createFromStdOut()
{
    char output[64];
    qint64 outputLength = process->readLine(output, sizeof(output));
    if (outputLength == -1)
        qCritical() << "Embedded application had unexpected output, which could not be interpreted as a window ID.";
    else {
#ifdef DEBUG_MULTIMEDIA
        qDebug() << "Trying to create embedded window with id from program standard output:" << output;
#endif
        QString winIdString = QString(output);
        bool success;
        WId wid = WId(winIdString.toLongLong(&success, 10));
        if (success && wid!=0)
            create(wid);
        else
            qCritical() << "Could not interpret output as window id";
    }
}

void EmbedApp::create(const WId wid)
{
    window = QWindow::fromWinId(wid);
    if (window != nullptr) {
        // Without the following two lines, key events are sometimes not sent to the embedded window:
        window->show();
        window->hide();
        // Turn the window into a widget, which can be embedded in the presentation (or control) window:
        widget = QWidget::createWindowContainer(window, static_cast<QWidget*>(parent()));
        emit widgetReady(this);
    }
}

QPair<int,int> EmbedApp::getNextLocation(int const page) const
{
    int idx = pages.indexOf(page);
    if (idx == -1) {
        int dist=-1000000, diff;
        for (int i=0; i<pages.size(); i++) {
            if (pages[i] == page) {
                idx = i;
                break;
            }
            diff = pages[i] - page;
            if ((dist>0 && dist>diff) || (dist<0 && diff>dist)) {
                dist = diff;
                idx = i;
            }
        }
    }
    return {pages[idx], indices[idx]};
}

void EmbedApp::terminate()
{
    if (process != nullptr && process->state() == QProcess::Running)
        process->terminate();
}
