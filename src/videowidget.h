#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMouseEvent>
#include <QUrl>
#include <QDir>
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
    VideoWidget(Poppler::MovieObject * movie, QWidget * parent = nullptr);
    ~VideoWidget();

protected:
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    QMediaPlayer * player;

public slots:
    void play();

private slots:
    void showPosterImage(QMediaPlayer::State state);
    void bouncePalindromeVideo(QMediaPlayer::State state);
    void restartVideo(QMediaPlayer::State state);
};

#endif // VIDEOWIDGET_H
