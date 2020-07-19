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
#include <cmath>

#include "cachemap.h"

CacheMap::~CacheMap()
{
    cacheThread->requestInterruption();
    cacheThread->wait(10000);
    if (cacheThread->isRunning()) {
        cacheThread->terminate();
        cacheThread->wait(10000);
    }
    delete cacheThread;
    qDeleteAll(data);
    data.clear();
}

qint64 CacheMap::setPixmap(int const page, QPixmap const* pix)
{
    // Check whether the pixmap is empty.
    if (pix->isNull())
        return 0;
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!pix->save(&buffer, "PNG")) {
        qWarning() << "Rendering failed." << this;
        delete bytes;
        return 0;
    }
    qint64 currentSize = qint64(bytes->size());
    if (data.contains(page) && data[page] != nullptr) {
        // Usually this should not happen.
        currentSize -= data[page]->size();
        delete data[page];
    }
    data[page] = bytes;
    return currentSize;
}

void CacheMap::clearCache()
{
#ifdef DEBUG_CACHE
    qDebug() << "Clear cache" << this << parent();
#endif
    qDeleteAll(data);
    data.clear();
}

void CacheMap::changeResolution(const double res)
{
    if (res == resolution)
        return;
#ifdef DEBUG_CACHE
    qDebug() << "Change resolution" << res << resolution << this << parent();
#endif
    clearCache();
    resolution = res;
}

QPixmap const CacheMap::getCachedPixmap(int const page) const
{
#ifdef DEBUG_CACHE
    qDebug() << "get cached page" << page << this << data.contains(page);
#endif
    QPixmap pixmap;
    if (data.contains(page))
        pixmap.loadFromData(*data.value(page), "PNG");
    return pixmap;
}

QPixmap const CacheMap::getPixmap(int const page)
{
#ifdef DEBUG_CACHE
    qDebug() << "get page" << page << this << data.contains(page);
#endif
    QPixmap pixmap;
    if (data.contains(page) && data.value(page) != nullptr) {
        pixmap.loadFromData(*data.value(page), "PNG");
        // Check whether pixmap has the correct size.
        QSizeF pageSize = resolution*pdf->getPageSize(page);
        if (pagePart != FullPage)
            pageSize.setWidth(pageSize.width()/2);
        if (std::abs(pixmap.height() - pageSize.height()) < 2 && std::abs(pixmap.width() - pageSize.width()) < 2)
            return pixmap;
#ifdef DEBUG_CACHE
        qDebug() << "Size changed:" << pixmap.size() << pageSize;
#endif
        // The size was wrong. Delete the old cached page.
        emit cacheSizeChanged(-data[page]->size());
        data.remove(page);
    }
    if (resolution <= 0.)
        return pixmap;
    if (renderCommand.isEmpty()) {
        pixmap = renderPixmap(page);
        emit cacheSizeChanged(setPixmap(page, &pixmap));
    }
    else {
        ExternalRenderer* renderer = new ExternalRenderer(page);
#if QT_VERSION_MAJOR <= 5 and QT_VERSION_MINOR < 15
        renderer->start(getRenderCommand(page));
#else
        QStringList renderCommandSplit = QProcess::splitCommand(getRenderCommand(page));
        renderer->start(renderCommandSplit.takeFirst(), renderCommandSplit);
#endif
        QByteArray const* bytes = nullptr;
        if (renderer->waitForFinished(60000))
            bytes = renderer->getBytes();
        else
            renderer->kill();
        delete renderer;
        pixmap.loadFromData(*bytes, "PNG");
        if (pagePart == FullPage) {
            data[page] = bytes;
            emit cacheSizeChanged(bytes->size());
        }
        else {
            delete bytes;
            if (pagePart == LeftHalf)
                pixmap = pixmap.copy(0, 0, pixmap.width()/2, pixmap.height());
            else
                pixmap = pixmap.copy(pixmap.width()/2, 0, pixmap.width()/2, pixmap.height());
            emit cacheSizeChanged(setPixmap(page, &pixmap));
        }
    }
    return pixmap;
}

qint64 CacheMap::clearPage(const int page)
{
    if (!data.contains(page))
        return 0;
    qint64 pageSize(data[page]->size());
    delete data[page];
    data.remove(page);
    return pageSize;
}

void CacheMap::receiveBytes()
{
    QByteArray const* bytes = cacheThread->getBytes();
    if (bytes != nullptr && !bytes->isEmpty()) {
        qint64 size_diff = bytes->size();
        int const page = cacheThread->getPage();
        if (data.contains(page))
            size_diff -= data[page]->size();
        data[page] = bytes;
        emit cacheSizeChanged(size_diff);
    }
#ifdef DEBUG_CACHE
    qDebug() << "Cache thread finished:" << this << parent();
#endif
    emit cacheThreadFinished();
}

bool CacheMap::updateCache(int const page)
{
    if (resolution <= 0.)
        return false;
    if (data.contains(page))
        return false;
    cacheThread->setPage(page);
    cacheThread->start();
    return true;
}

qint64 CacheMap::getSizeBytes() const
{
    qint64 size = 0;
    for (QMap<int, QByteArray const*>::const_iterator it=data.cbegin(); it!=data.cend(); it++)
        size += (*it)->size();
    return size;
}
