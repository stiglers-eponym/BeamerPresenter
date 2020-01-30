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

class VideoWidget : public QObject
{
    Q_OBJECT

public:
    VideoWidget(Poppler::MovieAnnotation const * annotation, QString const& urlSplitCharacter = "", QWidget* parent = nullptr);
    ~VideoWidget();
    Poppler::MovieAnnotation const * getAnnotation() const {return annotation;}
    qint64 getDuration() const {return player->duration();}
    qint64 getPosition() const {return player->position();}
    QMediaPlayer* getPlayer() {return player;}
    QMediaPlayer::State state() const {return player->state();}
    bool getAutoplay() const {return autoplay;}
    QString const& getUrl() const {return filename;}
    void setMute(bool const mute) {player->setMuted(mute);}
    void setGeometry(QRect const& rect);
    void setGeometry(int const x, int const y, int const w, int const h);
    //void raise() {view->raise();}
    void lower() {view->lower();}
    void show() {view->show();}
    void hide() {view->hide();}

private:
    QGraphicsScene* scene;
    QGraphicsView* view;
    QMediaPlayer* player;
    QGraphicsVideoItem* item;
    QGraphicsPixmapItem* pixmap = nullptr;
    QMediaPlaylist* playlist;
    QImage posterImage;
    QString filename;
    bool autoplay = false;
    Poppler::MovieAnnotation const* annotation;
    bool ownsScene = true;
    QUrl url;

public slots:
    void play();
    void pause() {player->pause();}
    void pausePosition(quint64 const position);
    void setPosition(qint64 const position) {player->setPosition(position);}

private slots:
    void showPosterImage(QMediaPlayer::MediaStatus status);
    void bouncePalindromeVideo(QMediaPlayer::MediaStatus status);
    void restartVideo(QMediaPlayer::MediaStatus status);

signals:
    void positionChanged(qint64 const position);
    void durationChanged(qint64 const position);
    void sendPlay();
    void sendPause();
    void sendPausePos(quint64 const position);
};

#endif // VIDEOWIDGET_H
