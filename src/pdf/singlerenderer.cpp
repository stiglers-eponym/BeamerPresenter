/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2020  stiglers-eponym

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

#include "singlerenderer.h"

SingleRenderer::~SingleRenderer()
{
    cacheThread->requestInterruption();
    cacheThread->wait(10000);
    cacheThread->exit();
    delete cacheThread;
    delete data;
}

void SingleRenderer::receiveBytes()
{
    data = cacheThread->getBytes();
    page = cacheThread->getPage();
    emit cacheThreadFinished();
}

void SingleRenderer::renderPage(const int page)
{
    if (page != this->page) {
        delete data;
        data = nullptr;
        cacheThread->setPage(page);
        cacheThread->start();
    }
}

QPixmap const SingleRenderer::getPixmap()
{
    QPixmap pixmap;
    pixmap.loadFromData(*data, "PNG");
    return pixmap;
}
