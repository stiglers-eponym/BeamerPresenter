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

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QtDebug>
#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QUrl>
#include <QDir>
#include <QImage>
#include <poppler-qt5.h>

/// "Widget-like" object showing video on slides.
/// VideoWidget contains a QGraphicsScene and everything required to show it,
/// but aims a behaving like a regular QVideoWidget. The reason for using a
/// QGraphicsSCene is that transparent drawing on top of the video is required.
///
/// Note: The structure of showing videos will hopefully change in the future.
class VideoWidget : public QObject
{
    Q_OBJECT

public:
    /// Construct the QGraphicsView and its content.
    VideoWidget(Poppler::MovieAnnotation const* annotation, QString const& urlSplitCharacter = "", QWidget* parent = nullptr);
    ~VideoWidget();
    Poppler::MovieAnnotation const* getAnnotation() const {return annotation;}
    qint64 getDuration() const {return player->duration();}
    qint64 getPosition() const {return player->position();}
    QMediaPlayer* getPlayer() {return player;}
    QMediaPlayer::State state() const {return player->state();}
    /// Values for autoplay: +1 if autoplay is explicitly enabled, -1 if it is explicitly disabled, 0 otherwise.
    signed char getAutoplay() const {return autoplay;}
    QString const& getUrl() const {return filename;}
    Poppler::MovieObject::PlayMode getPlayMode() const {return annotation->movie()->playMode();}
    void setMute(bool const mute) {player->setMuted(mute);}
    void setGeometry(QRect const& rect);
    void setGeometry(int const x, int const y, int const w, int const h);
    /// Move widget to bottom in stacking order among siblings.
    void lower() {view->lower();}
    //void raise() {view->raise();}

    /// Show widget if file name is valid and playlist has not produced any error.
    void show();
    /// Hide QGraphicsView widget.
    void hide() {view->hide();}

private:
    /// Graphics scene containing the video.
    QGraphicsScene* scene;
    /// Graphics view showing the scene.
    QGraphicsView* view;
    /// Media player for the video.
    QMediaPlayer* player;
    /// Playlist containing the video.
    QMediaPlaylist* playlist;
    /// Graphics video item containing the video in the graphics scene.
    QGraphicsVideoItem* item;
    /// TODO: this is broken. This should show the poster image.
    QGraphicsPixmapItem* pixmap = nullptr;
    /// Poster image: image shown instead of the video before and after playing the video.
    QImage posterImage;
    /// Path to the video file.
    QString filename;
    /// Autoplay: +1 if autoplay is explicitly enabled, -1 if it is explicitly disabled.
    signed char autoplay = 0;
    /// MovieAnnotation containing all available information about the video.
    Poppler::MovieAnnotation const* annotation;

public slots:
    void play();
    void pause() {player->pause();}
    /// Pause video and set position to given position.
    void pausePosition(quint64 const position);
    void setPosition(qint64 const position) {player->setPosition(position);}

private slots:
    /// This should show the poster image of the video if status in (LoadingMedia, EndOfMedia).
    /// But since that currently does not work, it instead hides the widget.
    void showPosterImage(QMediaPlayer::MediaStatus status);
    /// Invert playback rate for videos in palindrome mode when approaching the end (or beginning) of the video.
    void bouncePalindromeVideo(qint64 const position);

signals:
    /// position is measured in ms.
    void positionChanged(qint64 const position);
    /// duration is measured in ms.
    void durationChanged(qint64 const position);
    void sendPlay();
    void sendPause();
    /// Send pause signal along with current postion (in ms).
    void sendPausePos(quint64 const position);
};

#endif // VIDEOWIDGET_H
