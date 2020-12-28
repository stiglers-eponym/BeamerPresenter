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

#ifndef EMBEDAPP_H
#define EMBEDAPP_H

#include <QtDebug>
#include <QWidget>
#include <QWindow>
#include <QProcess>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif
#include <QTimer>

/// "widget-like" object for X-embedding an external application.
/// This works only in X (QT_QPA_PLATFORM == xcb).
class EmbedApp : public QObject
{
    Q_OBJECT

public:
    explicit EmbedApp(QStringList const& command, QString const& pid2wid, int const page, int const index, QWidget* parent = nullptr);
    ~EmbedApp();
    /// Add a location (page number and index among the embedded applications on that page) for this.
    void addLocation(int const page, int const index) {pages.append(page); indices.append(index);}
    /// Start the embedded application. Call the external application and prepare for embedding it.
    void start();
    /// Update the widget if it is visible.
    void update();
    /// Terminate the process of the embedded application.
    void terminate();
    QWidget* getWidget() {return widget;}
    QWindow* getWindow() {return window;}
    QProcess* getProcess() {return process;}
    bool isStarted() const {return process != nullptr;}
    bool isReady() const {return widget != nullptr;}
    QStringList const& getCommand() const {return command;}
    bool isOnPage(int const page) const {return pages.contains(page);}
    /// Return the next location in the presentation relativ to page, where this application appears.
    QPair<int,int> getNextLocation(int const page) const;

private:
    /// Called when the embedded application exits. Clean up by deleting the widget.
    void clearProcess(int const exitCode, QProcess::ExitStatus const exitStatus);
    /// Start pid2widProcess to get the window ID from the process ID.
    void getWidFromPid();
    /// Take control over the window and tell parent (PageWidget), that the widget is ready.
    void create(WId const wid);
    /// Minimum time in ms, after which pid2wid is called for the first time
    int const minDelayPidWidCaller = 100;
    /// Process of pid2wid: a script which tries to get the window id of the external application which should be embedded.
    QProcess* pid2widProcess = nullptr;
    /// Timer for calling pid2wid.
    QTimer* pid2widTimer = nullptr;
    /// Command for starting the external application.
    QStringList command;
    /// Pages in PDF which contain this embedded application.
    QList<int> pages;
    /// Index of this embedded application on each of the pages.
    /// Only relevant if a page contains multiple embedded applications (which is untested).
    QList<int> indices;
    /// Window in which the external application is embedded.
    QWindow* window = nullptr;
    /// Widget container containing the window.
    QWidget* widget = nullptr;
    QProcess* process = nullptr;
    /// Command for pid2wid: an external program (script) returning the window ID for a given process ID.
    QString pid2wid;
    /// Window ID of the embedded application.
    WId wid;

signals:
    /// The application is embedded and ready to be shown.
    void widgetReady(EmbedApp* embedApp);

private slots:
    /// Called when pid2widProcess finishes. This reads the window ID from pid2widProcess.
    /// If a valid window ID is found, create the embedded application (i.e. embed the existing application).
    /// Otherwise start pid2widTimer (with a longer interval).
    void receiveWidFromPid(int const exitCode, QProcess::ExitStatus const status);
    /// Try to read window ID from standard output of process.
    /// This function is called if process writes to standard output and pid2wid is not set.
    void createFromStdOut();
};

#endif // EMBEDAPP_H
