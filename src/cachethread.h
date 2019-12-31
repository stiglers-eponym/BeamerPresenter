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

#ifndef CACHETHREAD_H
#define CACHETHREAD_H

#include <QObject>
#include <QThread>
#include <QPixmap>
#include "externalrenderer.h"

class CacheMap;

class CacheThread : public QThread
{
    Q_OBJECT

private:
    int newPage = 0;
    int page = 0;
    CacheMap const* cacheMap;
    QByteArray const* bytes = nullptr;

public:
    CacheThread(QObject* parent = nullptr) : QThread(parent), cacheMap(nullptr) {}
    CacheThread(CacheMap const* cache, QObject* parent = nullptr) : QThread(parent), cacheMap(cache) {}
    void setCacheMap(CacheMap const* cache) {cacheMap = cache;}
    void setPage(int const pageNumber) {newPage = pageNumber;}
    /// This must be called exactly once after run() finished.
    QByteArray const* getBytes();
    int getPage() const {return page;}
    void run() override;

//signals:
    //void resultsReady(int const page, QByteArray const* pres);
};

#endif // CACHETHREAD_H
