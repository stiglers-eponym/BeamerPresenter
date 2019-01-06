/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMouseEvent>
#include <QUrl>
#include <QDir>
#include <QImage>
#include <QBrush>
#include <poppler-qt5.h>
#include <QVideoWidget>
#include <iostream>
#include <QKeyEvent>
#include <QMouseEvent>

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
