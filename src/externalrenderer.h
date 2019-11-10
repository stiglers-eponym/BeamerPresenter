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

#ifndef EXTERNALRENDERER_H
#define EXTERNALRENDERER_H

#include <QtDebug>
#include <QProcess>

class ExternalRenderer : public QProcess
{
    Q_OBJECT

public:
    ExternalRenderer(int const page, QObject* parent = nullptr);
    QByteArray const* getBytes() const {return bytes;}

private:
    int page;
    QByteArray const* bytes = nullptr;

protected slots:
    void returnImage(int const exitCode, QProcess::ExitStatus const exitStatus);

signals:
    void sendImage(QByteArray const* bytes, int const page);
};

#endif // EXTERNALRENDERER_H
