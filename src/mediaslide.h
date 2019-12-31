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

#ifndef MEDIASLIDE_H
#define MEDIASLIDE_H

#include "previewslide.h"
#include "videowidget.h"
#ifdef EMBEDDED_APPLICATIONS_ENABLED
#include "embedapp.h"
#endif
#include <QSlider>
#include <QTimer>

class MediaSlide : public PreviewSlide
{
    friend void connectVideos(MediaSlide*, MediaSlide*);
    Q_OBJECT
public:
    explicit MediaSlide(QWidget* parent=nullptr);
    explicit MediaSlide(PdfDoc const * const document, int const pageNumber, PagePart const part, QWidget* parent=nullptr);
    ~MediaSlide() override {clearAll();}
    void renderPage(int pageNumber, bool const hasDuration);
    void setCacheVideos(bool const cacheThem) {cacheVideos=cacheThem;}
    void setMultimediaSliders(QList<QSlider*> sliderList);
    void setAutostartDelay(double const delay) {autostartDelay=delay;}
    bool hasActiveMultimediaContent() const;
    void updateCacheVideos(int const pageNumber);
    double getAutostartDelay() const {return autostartDelay;}
    int getSliderNumber() const {return videoSliders.size()+soundSliders.size()+soundLinkSliders.size();}
    bool isMuted() const {return mute;}
    QMap<int,QSlider*> const& getVideoSliders() {return videoSliders;}
    virtual void clearAll() override;
    void setMuted(bool muted);
    void showAllWidgets();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    void avoidMultimediaBug(); // TODO: fix this!
    void startAllEmbeddedApplications(int const index);
    void initEmbeddedApplications(int const pageNumber);
    void setEmbedFileList(const QStringList& files) {embedFileList=files;}
    void closeEmbeddedApplications(int const index);
    void closeAllEmbeddedApplications();
    void setAutostartEmbeddedDelay(double const delay) {autostartEmbeddedDelay=delay;}
    void setPid2Wid(QString const & program) {pid2wid=program;}
#endif

protected:
    bool mute = false;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void followHyperlinks(QPoint const& pos);
    virtual void clearLists();
    QList<VideoWidget*> videoWidgets;
    QList<VideoWidget*> cachedVideoWidgets;
    QList<QRect> videoPositions;
    QList<QMediaPlayer*> soundPlayers;
    QList<QRect> soundPositions;
    QMap<int,QMediaPlayer*> soundLinkPlayers;
    QMap<int,QSlider*> videoSliders;
    QMap<int,QSlider*> soundSliders;
    QMap<int,QSlider*> soundLinkSliders;
    QTimer* const autostartTimer = new QTimer(this);
    double autostartDelay = -1.; // delay for starting multimedia content in s
    bool cacheVideos = true;
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    QString pid2wid;
    QTimer* const autostartEmbeddedTimer = new QTimer(this);
    QList<EmbedApp*> embedApps;
    QMap<int,QMap<int,int>> embedMap;
    QList<QRect> embedPositions;
    QStringList embedFileList;
    double autostartEmbeddedDelay = -1.; // delay for starting embedded applications in s
#endif
    virtual void animate(int const = -1) {}
    virtual void stopAnimation() {}
    virtual void setDuration() {}

public slots:
    void pauseAllMultimedia();
    void startAllMultimedia();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    void receiveEmbedApp(EmbedApp* app);
#endif

signals:
    void requestMultimediaSliders(int const n);
};

#endif // MEDIASLIDE_H
