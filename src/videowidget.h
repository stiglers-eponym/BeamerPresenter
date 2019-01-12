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

#include <iostream>
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QUrl>
#include <QDir>
#include <QImage>
#include <QBrush>
#include <QMouseEvent>
#include <poppler-qt5.h>

class VideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    VideoWidget(Poppler::MovieAnnotation const * annotation, QWidget * parent = nullptr);
    ~VideoWidget();
    QMediaPlayer::State state() const;
    Poppler::MovieAnnotation const * getAnnotation() const;
    qint64 getDuration() const;
    QMediaPlayer const * getPlayer() const;

protected:
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    QMediaPlayer * player;
    QImage posterImage;
    Poppler::MovieAnnotation const * annotation;

public slots:
    void play();
    void pause();
    void setPosition(qint64 const position);

private slots:
    void showPosterImage(QMediaPlayer::State state);
    void bouncePalindromeVideo(QMediaPlayer::State state);
    void restartVideo(QMediaPlayer::State state);

signals:
    void positionChanged(qint64 const position);
    void durationChanged(qint64 const position);
};

#endif // VIDEOWIDGET_H
