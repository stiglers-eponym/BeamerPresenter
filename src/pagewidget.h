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

#ifndef PAGE_H
#define PAGE_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QPainter>
#include <QTimer>
#include <QSlider>
#include <QMouseEvent>
#include <QBuffer>
#include <QDesktopServices>
#include <QMediaPlayer>
#include <poppler-qt5.h>
#include "videowidget.h"
#include "embedapp.h"
#include "enumerates.h"

class PageWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    PageWidget(QWidget* parent);
    PageWidget(Poppler::Page* page, QWidget* parent);
    ~PageWidget() override;
    void renderPage(Poppler::Page* page, bool const setDuration=true, QPixmap const* pixmap=nullptr);
    long int updateCache(Poppler::Page const* page);
    long int updateCache(QPixmap const* pixmap, int const index);
    long int updateCache(QByteArray const* pixmap, int const index);
    long int clearCachePage(int const index);
    void updateCacheVideos(Poppler::Page const* page);
    void clearCache();
    void clearAll();
    void startAllEmbeddedApplications(int const index);
    void initEmbeddedApplications(Poppler::Page const* page);
    void avoidMultimediaBug();

    void setMultimediaSliders(QList<QSlider*> sliderList);
    void setPresentationStatus(bool const status);
    void setShowMultimedia(bool const showMultimedia) {this->showMultimedia=showMultimedia;}
    void setUrlSplitCharacter(QString const& splitCharacter) {urlSplitCharacter=splitCharacter;}
    void setPagePart(PagePart const state) {pagePart=state;}
    void setEmbedFileList(const QStringList& files) {embedFileList=files;}
    void setUseCache(bool const use) {useCache=use;}
    void setCacheVideos(bool const cache) {cacheVideos=cache;}
    void setBackground(QBrush bg) {background=bg;}

    long int getCacheSize() const;
    int getCacheNumber() const {return cache.size();}
    QPixmap getPixmap(Poppler::Page const* page) const;
    QPixmap const getCache(int const index) const;
    QByteArray const* getCachedBytes(int const index) const;
    int pageNumber() const {return pageIndex;}
    Poppler::Page* getPage() {return page;}
    double getDuration() const {return duration;}
    bool cacheContains(int const index) const {return cache.contains(index);}
    bool hasActiveMultimediaContent() const;

private:
    void clearLists();
    QList<Poppler::Link*> links;
    QList<QRect> linkPositions;
    QList<VideoWidget*> videoWidgets;
    QList<VideoWidget*> cachedVideoWidgets;
    QList<QRect> videoPositions;
    QList<QMediaPlayer*> soundPlayers;
    QList<QRect> soundPositions;
    QMap<int,QMediaPlayer*> soundLinkPlayers;
    QMap<int,QSlider*> videoSliders;
    QMap<int,QSlider*> soundSliders;
    QMap<int,QSlider*> soundLinkSliders;
    QMap<int,QMap<int,int>> embedMap;
    QList<EmbedApp*> embedApps;
    QList<QRect> embedPositions;
    QTimer* const autostartTimer = new QTimer(this);
    QTimer* const autostartEmbeddedTimer = new QTimer(this);
    QSize oldSize = QSize();
    QStringList embedFileList;
    QString pid2wid;
    QString urlSplitCharacter = "";
    double autostartDelay = -1.; // delay for starting multimedia content in s
    double autostartEmbeddedDelay = -1.; // delay for starting embedded applications in s
    bool cacheVideos = true;

protected:
    virtual void animate() {}
    virtual void endAnimation() {}
    virtual void setDuration() {}
    QBrush background = QBrush(QColor(0,0,0));
    Poppler::Page* page = nullptr;
    QMap<int,QByteArray const*> cache;
    PagePart pagePart = FullPage; // Which part of the page is shown on this label
    int shiftx;
    int shifty;
    QPixmap pixmap;
    double resolution; // resolution in pixels per point = dpi/72
    double duration; // duration of the current page in s
    int pageIndex = 0; // page number
    bool showMultimedia = false;
    bool useCache = true;
    bool pointer_visible = true;

    //void resizeGL(int w, int h) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    virtual void paintGL() override;

public slots:
    void togglePointerVisibility();
    void pauseAllMultimedia();
    void startAllMultimedia();
    void receiveEmbedApp(EmbedApp* app);
    void setAutostartDelay(double const delay) {autostartDelay=delay;}
    void setAutostartEmbeddedDelay(double const delay) {autostartEmbeddedDelay=delay;}
    virtual void setAnimationDelay(int const delay_ms) {}
    void setPid2Wid(QString const & program) {pid2wid=program;}

signals:
    void sendNewPageNumber(int const pageNumber);
    void requestMultimediaSliders(int const n);
    void sendCloseSignal();
    void focusPageNumberEdit();
    void timeoutSignal();
    void sendShowFullscreen();
    void sendEndFullscreen();
};

#endif // PAGE_H
