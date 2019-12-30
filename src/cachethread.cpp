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

#include "cachethread.h"
#include "cachemap.h"


void CacheThread::run()
{
    // Handle one page. This page should not change while rendering.
    int const page = this->page;
    //qDebug() << "Cache page" << page << cacheMap << cacheMap->parent();
    QString renderCommand = cacheMap->getRenderCommand(page);
    if (renderCommand.isEmpty()) {
        QPixmap pixmap = cacheMap->renderPixmap(page);
        if (isInterruptionRequested())
            return;
        QByteArray* bytes = new QByteArray();
        QBuffer buffer(bytes);
        buffer.open(QIODevice::WriteOnly);
        if (!pixmap.save(&buffer, "PNG"))
            return;
        if (isInterruptionRequested())
            delete bytes;
        else
            emit resultsReady(page, bytes);
    }
    else {
        ExternalRenderer* renderer = new ExternalRenderer(page);
        renderer->start(renderCommand);
        if (!renderer->waitForFinished(60000)) {
            renderer->kill();
            delete renderer;
            return;
        }
        QByteArray const* bytes = renderer->getBytes();
        delete renderer;
        if (cacheMap->getPagePart() == FullPage) {
            if (bytes != nullptr)
                emit resultsReady(page, bytes);
            return;
        }
        if (isInterruptionRequested())
            return;
        QPixmap pixmap;
        pixmap.loadFromData(*bytes, "PNG");
        delete bytes;
        if (cacheMap->getPagePart() == LeftHalf)
            pixmap = pixmap.copy(0, 0, pixmap.width()/2, pixmap.height());
        else
            pixmap = pixmap.copy(pixmap.width()/2, 0, pixmap.width()/2, pixmap.height());
        QByteArray* newBytes = new QByteArray();
        QBuffer buffer(newBytes);
        buffer.open(QIODevice::WriteOnly);
        if (!pixmap.save(&buffer, "PNG"))
            return;
        if (isInterruptionRequested()) {
            delete newBytes;
            return;
        }
        emit resultsReady(page, newBytes);
    }
}
