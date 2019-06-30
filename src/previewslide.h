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

#ifndef PREVIEWSLIDE_H
#define PREVIEWSLIDE_H

#include <QBuffer>
#include <QMouseEvent>
#include "basicslide.h"

class PreviewSlide : public BasicSlide
{
    Q_OBJECT
public:
    explicit PreviewSlide(QWidget* parent=nullptr) : BasicSlide(parent) {}
    explicit PreviewSlide(Poppler::Page* page, QWidget* parent=nullptr) : BasicSlide(parent) {renderPage(page);}
    ~PreviewSlide() override;
    virtual void renderPage(Poppler::Page* page, QPixmap const* pixmap=nullptr) override;
    long int updateCache(Poppler::Page const* page);
    long int updateCache(QPixmap const* pixmap, int const index);
    long int updateCache(QByteArray const* pixmap, int const index);
    long int clearCachePage(int const index);
    void updateCacheVideos(Poppler::Page const* page);
    void clearCache();
    void setUseCache(bool const use) {useCache=use;}
    long int getCacheSize() const;
    int getCacheNumber() const {return cache.size();}
    QPixmap getPixmap(Poppler::Page const* page) const;
    QPixmap const getCache(int const index) const;
    QByteArray const* getCachedBytes(int const index) const;
    bool cacheContains(int const index) const {return cache.contains(index);}
    void setUrlSplitCharacter(QString const& splitCharacter) {urlSplitCharacter=splitCharacter;}

protected:
    QList<Poppler::Link*> links;
    QList<QRect> linkPositions;
    QMap<int,QByteArray const*> cache;
    QSize oldSize;
    QString urlSplitCharacter = "";
    bool useCache = true;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
};

#endif // PREVIEWSLIDE_H
