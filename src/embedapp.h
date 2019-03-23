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
#include <QTimer>

class EmbedApp : public QObject
{
    Q_OBJECT

public:
    explicit EmbedApp(QStringList const& command, QString const& pid2wid, int const page, int const index, QWidget* parent = nullptr);
    ~EmbedApp();
    void addLocation(int const page, int const index) {pages.append(page); indices.append(index);}
    QWidget* getWidget() {return widget;}
    QWindow* getWindow() {return window;}
    QProcess* getProcess() {return process;}
    bool isStarted() const {return process!=nullptr;}
    bool isReady() const {return widget!=nullptr;}
    QStringList const& getCommand() const {return command;}
    bool isOnPage(int const page) const {return pages.contains(page);}
    int* getNextLocation(int const page) const;
    void start();

private:
    void clearProcess(int const exitCode, QProcess::ExitStatus const exitStatus);
    void getWidFromPid();
    void create(WId const wid);
    int const minDelayPidWidCaller = 50; // Minimum time in ms, after which pid2wid is called for the first time
    QProcess* pid2widProcess = nullptr;
    QTimer* pid2widTimer = nullptr;
    QStringList command;
    QList<int> pages;
    QList<int> indices;
    QWindow* window = nullptr;
    QWidget* widget = nullptr;
    QProcess* process = nullptr;
    QString pid2wid;
    WId wid;

signals:
    void widgetReady(EmbedApp* embedApp);

private slots:
    void receiveWidFromPid(int const exitCode);
    void createFromStdOut();
};

#endif // EMBEDAPP_H
