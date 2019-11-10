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

#include "externalrenderer.h"

ExternalRenderer::ExternalRenderer(int const page, QObject* parent) : QProcess(parent)
{
    this->page = page;
    connect(this, QOverload<int const, QProcess::ExitStatus const>::of(&QProcess::finished), this, &ExternalRenderer::returnImage);
}

void ExternalRenderer::returnImage(int const exitCode, QProcess::ExitStatus const exitStatus)
{
    if (exitStatus != 0 || exitCode != 0) {
        qWarning() << "Call to external renderer failed, exit code" << exitCode;
        return;
    }
    bytes = new QByteArray(readAllStandardOutput());
    emit sendImage(bytes, page);
}
