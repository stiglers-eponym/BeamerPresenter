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

CacheThread::~CacheThread()
{
    // Old bytes which have not been picked up could still be around. Delete them.
    delete bytes;
    bytes = nullptr;
}

void CacheThread::run()
{
    // Handle one page. This page should not change while rendering.
    page = newPage;
    QString renderCommand = master->getRenderCommand(page);
    if (renderCommand.isEmpty()) {
        QPixmap pixmap = master->renderPixmap(page);
        if (isInterruptionRequested())
            return;
        QByteArray* bytes_nonconst = new QByteArray();
        QBuffer buffer(bytes_nonconst);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        // Usually bytes==nullptr. But if the old bytes have not been picked up, we should delete them here.
        delete bytes;
        bytes = bytes_nonconst;
    }
    else {
        ExternalRenderer* renderer = new ExternalRenderer(page);
        renderer->start(renderCommand);
        if (!renderer->waitForFinished(60000)) {
            renderer->kill();
            // Usually bytes==nullptr. But if the old bytes have not been picked up, we should delete them here.
            delete bytes;
            bytes = nullptr;
            delete renderer;
            return;
        }
        // Usually bytes==nullptr. But if the old bytes have not been picked up, we should delete them here.
        delete bytes;
        bytes = renderer->getBytes();
        delete renderer;
        if (master->getPagePart() != FullPage) {
            if (isInterruptionRequested()) {
                delete bytes;
                bytes = nullptr;
                return;
            }
            QPixmap pixmap;
            pixmap.loadFromData(*bytes, "PNG");
            delete bytes;
            if (master->getPagePart() == LeftHalf)
                pixmap = pixmap.copy(0, 0, pixmap.width()/2, pixmap.height());
            else
                pixmap = pixmap.copy(pixmap.width()/2, 0, pixmap.width()/2, pixmap.height());
            QByteArray* bytes_nonconst = new QByteArray();
            QBuffer buffer(bytes_nonconst);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            bytes = bytes_nonconst;
        }
    }
}

QByteArray const* CacheThread::getBytes()
{
    // Return bytes and set bytes=nullptr.
    QByteArray const* returnBytes = bytes;
    bytes = nullptr;
    return returnBytes;
}
