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
#include "media/videowidget.h"
#ifdef EMBEDDED_APPLICATIONS_ENABLED
#include "media/embedapp.h"
#endif
#include <QGraphicsView>
#include <QSlider>
#include <QTimer>

class MediaSlide : public PreviewSlide
{
    friend void connectVideos(MediaSlide*, MediaSlide*);
    Q_OBJECT
public:
    explicit MediaSlide(QWidget* parent=nullptr);
    explicit MediaSlide(PdfDoc const * const document, PagePart const part, QWidget* parent=nullptr);
    ~MediaSlide() override {clearAll();}
    /// Clear all contents of the label.
    /// This function is called when the document is reloaded or the program is closed and everything should be cleaned up.
    virtual void clearAll() override;
    /// Show page on this widget.
    void renderPage(int pageNumber, bool const hasDuration);
    /// Enabel or disable pre-loading of videos.
    void setCacheVideos(bool const cacheThem) {cacheVideos=cacheThem;}
    /// Connect multimedia sliders to video widgets on this page.
    void setMultimediaSliders(QList<QSlider*> sliderList);
    /// Configure auto play of multimedia content.
    void setAutostartDelay(qreal const delay) {autostartDelay=delay;}
    /// Check whether the given position is contained in a link.
    bool hoverLink(QPoint const& pos) const;
    bool hasActiveMultimediaContent() const;
    /// Load videos on the given page and keep them in cache until the next slide change.
    void updateCacheVideos(int const pageNumber);
    qreal getAutostartDelay() const {return autostartDelay;}
    /// Total number of silders required by multimedia objects on this slide.
    int getSliderNumber() const {return videoSliders.size()+soundSliders.size()+soundLinkSliders.size();}
    bool isMuted() const {return mute;}
    QMap<int,QSlider*> const& getVideoSliders() {return videoSliders;}
    void setMuted(bool muted);
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    /// Start all embedded Applications on given page.
    void startAllEmbeddedApplications(int const index);
    void initEmbeddedApplications(int const pageNumber);
    /// Set list of files which should be opened as embedded applications when found in a link.
    void setEmbedFileList(QStringList const& files) {embedFileList=files;}
    /// Close all embedded applications of the given slide (slide number = index)
    void closeEmbeddedApplications(int const index);
    /// Close all embedded applications on all slides.
    void closeAllEmbeddedApplications();
    void setAutostartEmbeddedDelay(qreal const delay) {autostartEmbeddedDelay=delay;}
    /// Set external program translating a process ID to a window ID.
    void setPid2Wid(QString const& program) {pid2wid=program;}
#endif

protected:
    /// Clear page specific content.
    /// This function is called when going to an other page, which is not just an overlay of the previous page.
    /// It deletes all multimedia content associated with the current page.
    virtual void clearLists();
    /// Mouse release: handle links and annotations (including multimedia and embedded applications)
    void mouseReleaseEvent(QMouseEvent* event) override;
    /// Mouse move: change cursor when entering a link area.
    void mouseMoveEvent(QMouseEvent* event) override;
    /// Follow hyperlinks on the given position.
    void followHyperlinks(QPoint const& pos);
    /// List of video widgets on the current slide.
    QList<VideoWidget*> videoWidgets;
    /// List of video widgets cached for the next slide/
    QList<VideoWidget*> cachedVideoWidgets;
    /// List of positions of video widgets (same order as videoWidgets).
    QList<QRect> videoPositions;
    /// List of sound players for sound annotations on the current page.
    QList<QMediaPlayer*> soundPlayers;
    /// List of positions of sound annotations on the current page (same order as soundPlayers).
    QList<QRect> soundPositions;
    /// Map of link indices to media players for sounds. Sound links are handled independent of sound annotations.
    QMap<int, QMediaPlayer*> soundLinkPlayers;
    /// Sliders for video widgets: map indices of videoWidgets to sliders.
    QMap<int, QSlider*> videoSliders;
    /// Sliders for sound annotations: map indices of soundPlayers to sliders.
    QMap<int, QSlider*> soundSliders;
    /// Sliders for sound links: map link indices of sound links to sliders.
    QMap<int, QSlider*> soundLinkSliders;
    /// Timer for delayed auto start of multimedia content.
    QTimer* const autostartTimer = new QTimer(this);
    bool mute = false;
    /// delay for starting multimedia content in s. A negative value is treated as infinity.
    qreal autostartDelay = -1.;
    bool cacheVideos = true;
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    QString pid2wid;
    QTimer* const autostartEmbeddedTimer = new QTimer(this);
    /// List of embedded applications.
    QList<EmbedApp*> embedApps;
    /// Map page intex to (map link index on page to index of EmbedApp in embedApps).
    QMap<int, QMap<int,int>> embedMap;
    /// Positions (areas) of embedded applications (same order as embedApps).
    QList<QRect> embedPositions;
    /// List of applications which should be embedded when called by a link.
    QStringList embedFileList;
    /// delay for starting embedded applications in s. A negative value is treated as infinity.
    qreal autostartEmbeddedDelay = -1.;
#endif
    /// Called after rendering page but before loading showing multimedia content.
    /// This will be relevant in PresentationSlide.
    virtual void animate(int const = -1) {}
    /// Called at the beginning of renderPage. This will be relevant in PresentationSlide.
    virtual void stopAnimation() {}
    /// Called in renderPage. This will be relevant in PresentationSlide.
    virtual void setDuration() {}

public slots:
    void pauseAllMultimedia();
    void startAllMultimedia();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    /// Receive an embedded application and mark it in embedMap. Show the app if it should be visible.
    void receiveEmbedApp(EmbedApp* app);
#endif

signals:
    /// Request multimeida sliders from ControlScreen.
    void requestMultimediaSliders(int const n);
};

#endif // MEDIASLIDE_H
