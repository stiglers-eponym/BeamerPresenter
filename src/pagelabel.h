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

#include <QtDebug>
#include <QWidget>
#include <QWindow>
#include <QLabel>
#include <QProcess>
#include <QTimer>
#include <QSlider>
#include <QMouseEvent>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QDesktopServices>
#include <poppler-qt5.h>
#include "videowidget.h"
#include "mediaslider.h"
#include "embedapp.h"

enum PagePart {
    LeftHalf = 1,
    FullPage = 0,
    RightHalf = -1,
};

class PageLabel : public QLabel
{
    Q_OBJECT

public:
    PageLabel(QWidget* parent);
    PageLabel(Poppler::Page* page, QWidget* parent);
    ~PageLabel();
    void renderPage(Poppler::Page* page, bool const setDuration=true, QPixmap const* pixmap=nullptr);
    long int updateCache(Poppler::Page const* page);
    long int updateCache(QPixmap const* pixmap, int const index);
    long int updateCache(QByteArray const* pixmap, int const index);
    long int clearCachePage(int const index);
    void clearCache();
    void clearAll();
    void startAllEmbeddedApplications(int const index);
    void initEmbeddedApplications(Poppler::Page const* page);
    void avoidMultimediaBug();

    void setMultimediaSliders(QList<MediaSlider*> sliderList);
    void setPresentationStatus(bool const status) {isPresentation=status;}
    void setShowMultimedia(bool const showMultimedia) {this->showMultimedia=showMultimedia;}
    void setUrlSplitCharacter(QString const& splitCharacter) {urlSplitCharacter=splitCharacter;}
    void setPagePart(PagePart const state) {pagePart=state;}
    void setEmbedFileList(const QStringList& files) {embedFileList=files;}
    void setUseCache(bool const use) {useCache=use;}

    long int getCacheSize() const;
    int getCacheNumber() const {return cache.size();}
    QPixmap getPixmap(Poppler::Page const* page) const;
    QPixmap const* getCache(int const index) const;
    QByteArray const* getCachedBytes(int const index) const;
    int pageNumber() const {return pageIndex;}
    Poppler::Page* getPage() {return page;}
    double getDuration() const {return duration;}
    bool cacheContains(int const index) const {return cache.contains(index);}
    bool hasActiveMultimediaContent() const;

private:
    void clearLists();
    Poppler::Page* page = nullptr;
    QMap<int,QByteArray const*> cache;
    QList<Poppler::Link*> links;
    QList<QRect> linkPositions;
    QList<VideoWidget*> videoWidgets;
    QList<QRect> videoPositions;
    QList<QMediaPlayer*> soundPlayers;
    QList<QRect> soundPositions;
    QMap<int,QMediaPlayer*> soundLinkPlayers;
    QMap<int,MediaSlider*> videoSliders;
    QMap<int,MediaSlider*> soundSliders;
    QMap<int,MediaSlider*> soundLinkSliders;
    QMap<int,QMap<int,int>> embedMap;
    QList<EmbedApp*> embedApps;
    QList<QRect> embedPositions;
    QTimer* const autostartTimer = new QTimer(this);
    QTimer* const autostartEmbeddedTimer = new QTimer(this);
    QSize oldSize = QSize();
    QStringList embedFileList;
    QString pid2wid;
    QString urlSplitCharacter = "";
    PagePart pagePart = FullPage; // Which part of the page is shown on this label
    double resolution; // resolution in pixels per point = dpi/72
    double autostartDelay = -1.; // delay for starting multimedia content in s
    double autostartEmbeddedDelay = -1.; // delay for starting embedded applications in s
    double duration; // duration of the current page in s
    int minimumAnimationDelay = 40; // minimum frame time in ms
    int pageIndex = 0; // page number
    bool isPresentation = true;
    bool showMultimedia = true;
    bool useCache = true;
    bool pointer_visible = true;

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

public slots:
    void togglePointerVisibility();
    void pauseAllMultimedia();
    void startAllMultimedia();
    void receiveEmbedApp(EmbedApp* app);
    void setAutostartDelay(double const delay) {autostartDelay=delay;}
    void setAutostartEmbeddedDelay(double const delay) {autostartEmbeddedDelay=delay;}
    void setAnimationDelay(int const delay_ms) {minimumAnimationDelay=delay_ms;}
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
