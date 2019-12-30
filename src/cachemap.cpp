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

#include "cachemap.h"

CacheMap::~CacheMap()
{
    for (QMap<int, QByteArray const*>::const_iterator it=data.cbegin(); it!=data.cend(); it++)
        delete *it;
    data.clear();
    delete cacheTimer;
    // TODO: correctly kill thread
    delete cacheThread;
}

CacheMap::CacheMap(CacheMap& other) :
    QObject(other.parent()),
    data(other.data),
    size(other.size),
    pdf(other.pdf),
    resolution(other.resolution),
    pagePart(other.pagePart),
    renderer(other.renderer),
    cacheThread(other.cacheThread),
    cacheTimer(other.cacheTimer)
{
    if (cacheThread != nullptr)
        cacheThread->setCacheMap(this);
}

void CacheMap::setupCacheThread()
{
    cacheThread = new CacheThread(this, parent());
    cacheTimer = new QTimer(this);
    cacheTimer->setSingleShot(true);
    connect(cacheTimer, &QTimer::timeout, this, [&](){cacheTimer->start();}); // TODO: find a more elegant way.
    connect(cacheThread, &CacheThread::resultsReady, this, &CacheMap::receiveBytes);
}

/// Write the pixmap in png format to a QBytesArray at *value(page).
qint64 CacheMap::setPixmap(int const page, QPixmap const* pix)
{
    // Check whether the pixmap is empty.
    if (pix->isNull())
        return 0;
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    pix->save(&buffer, "PNG");
    data[page] = bytes;
    qint64 currentSize = qint64(bytes->size());
    size += currentSize;
    return currentSize;
}

void CacheMap::clearCache()
{
    for (QMap<int, QByteArray const*>::const_iterator it=data.cbegin(); it!=data.cend(); it++)
        delete *it;
    data.clear();
}

void CacheMap::changeResolution(const double res)
{
    if (res != resolution)
        clearCache();
    resolution = res;
}

QPixmap const CacheMap::getCachedPixmap(int const page) const
{
    QPixmap pixmap;
    if (data.contains(page))
        pixmap.loadFromData(*data.value(page), "PNG");
    return pixmap;
}

QPixmap const CacheMap::renderPixmap(int const page) const
{
    Poppler::Page const* cachePage = pdf->getPage(page);
    QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
    if (pagePart == FullPage)
        return QPixmap::fromImage(image);
    else if (pagePart == LeftHalf)
        return QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
    else
        return QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
}

QPixmap const CacheMap::getPixmap(int const page)
{
    QPixmap pixmap;
    if (data.contains(page))
        pixmap.loadFromData(*data.value(page), "PNG");
    else {
        pixmap = renderPixmap(page);
        setPixmap(page, &pixmap);
    }
    return pixmap;
}

QString const CacheMap::getRenderCommand(int const page) const
{
    if (renderer.isEmpty())
        return renderer;
    QString command = renderer;
    command.replace("%file", pdf->getPath());
    command.replace("%page", QString::number(page+1));
    if (pagePart==FullPage)
        command.replace("%width", QString::number(int(resolution*pdf->getPageSize(page).width()+0.5)));
    else
        command.replace("%width", QString::number(int(2*resolution*pdf->getPageSize(page).width()+0.5)));
    command.replace("%height", QString::number(int(resolution*pdf->getPageSize(page).height()+0.5)));
    return command;
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

void CacheMap::receiveBytes(int const page, QByteArray const* bytes)
{
    qint64 size_diff = bytes->size();
    if (data.contains(page))
        size_diff -= data[page]->size();
    data[page] = bytes;
    emit cacheSizeChanged(size_diff);
}

void CacheMap::updateCache(int const page)
{
    if (data.contains(page))
        return;
    cacheThread->setPage(page);
    cacheThread->start();
}
