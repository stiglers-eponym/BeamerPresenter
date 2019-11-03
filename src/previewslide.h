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
    explicit PreviewSlide(PdfDoc const * const document, int const pageNumber, QWidget* parent=nullptr);
    ~PreviewSlide() override;
    virtual void renderPage(int const pageNumber, QPixmap const* pix=nullptr) override;
    qint64 updateCache(int const pageNumber);
    qint64 updateCache(QPixmap const* pix, int const index);
    qint64 updateCache(QByteArray const* bytes, int const index);
    qint64 clearCachePage(int const index);
    virtual void clearCache();
    void setUseCache(char const use) {useCache=use;}
    qint64 getCacheSize() const;
    int getCacheNumber() const {return cache.size();}
    QPixmap const getPixmap(int const pageNumber) const;
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
    char useCache = 1; // 0=don't cache; 1=use cache with internal renderer; 2=use cache with external renderer
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    QPair<double,double> basicRenderPage(int const pageNumber, QPixmap const* pix);
};

#endif // PREVIEWSLIDE_H
