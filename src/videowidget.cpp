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

#include "videowidget.h"

VideoWidget::VideoWidget(Poppler::MovieAnnotation const* annotation, QString const& urlSplitCharacter, QWidget* parent) :
    QObject(parent),
    scene(new QGraphicsScene(this)),
    view(new QGraphicsView(scene, parent)),
    player(new QMediaPlayer(view, QMediaPlayer::VideoSurface)),
    item(new QGraphicsVideoItem),
    annotation(annotation)
{
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("border: 0px");
    scene->addItem(item);

    view->setMouseTracking(true);
    player->setVideoOutput(item);
    Poppler::MovieObject* movie = annotation->movie();
    if (movie->showPosterImage()) {
        QPalette* palette = new QPalette();
        posterImage = movie->posterImage();
        if (!posterImage.isNull())
            palette->setBrush( QPalette::Window, QBrush(posterImage));
        view->setPalette(*palette);
        delete palette;
        view->setAutoFillBackground(true);
    }

    filename = movie->url();
    QUrl url = QUrl(filename, QUrl::TolerantMode);
    QStringList splitFileName;
    if (!urlSplitCharacter.isEmpty()) {
        splitFileName = movie->url().split(urlSplitCharacter);
        url = QUrl(splitFileName[0], QUrl::TolerantMode);
        splitFileName.pop_front();
    }
    if (!url.isValid())
        url = QUrl::fromLocalFile(url.path());
    if (url.isRelative())
        url = QUrl::fromLocalFile(QDir(".").absoluteFilePath(url.path()));
    player->setMedia(url);
    if (splitFileName.contains("mute"))
        player->setMuted(true);
    // TODO: loop is broken in wayland.
    if (splitFileName.contains("loop"))
        connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoWidget::restartVideo);
    else {
        switch (movie->playMode())
        {
            case Poppler::MovieObject::PlayOpen:
                // TODO: PlayOpen should leave controls open (but they are not implemented yet).
            case Poppler::MovieObject::PlayOnce:
                connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoWidget::showPosterImage);
                break;
            case Poppler::MovieObject::PlayPalindrome:
                qWarning() << "WARNING: play mode=palindrome does not work as it should.";
                connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoWidget::bouncePalindromeVideo);
                break;
            case Poppler::MovieObject::PlayRepeat:
                connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoWidget::restartVideo);
                break;
        }
    }

    // TODO: video control bar
    // if (movie->showControls()) { TODO }

    // Scale the video such that it fits the widget size.
    // I like these results, but I don't know whether this is what other people would expect.
    // Please write me a comment if you would prefer an other way of handling the aspect ratio.
    item->setAspectRatioMode(Qt::IgnoreAspectRatio);
    if (splitFileName.contains("autostart"))
        autoplay = true;
    item->show();
}

VideoWidget::~VideoWidget()
{
    player->stop();
    player->disconnect();
    delete annotation;
    delete player;
    delete view;
    delete scene;
    // item is owned by scene.
}

void VideoWidget::play()
{
    if (player->mediaStatus()==QMediaPlayer::LoadingMedia || player->mediaStatus()==QMediaPlayer::EndOfMedia)
        player->bind(this);
    player->play();
}

void VideoWidget::pausePosition(quint64 const position)
{
    player->pause();
    player->setPosition(position);
}

void VideoWidget::showPosterImage(QMediaPlayer::MediaStatus status)
{
    if (status==QMediaPlayer::LoadingMedia || status==QMediaPlayer::EndOfMedia) {
        // Unbinding and binding this to the player is probably an ugly way of solving this.
        // TODO: find a better way of writing this.
        player->unbind(this);
        view->show();
    }
}

void VideoWidget::bouncePalindromeVideo(QMediaPlayer::MediaStatus status)
{
    // TODO: The result of this function is not what it should be.
    // But this could also depend on the encoding of the video.
    if (status==QMediaPlayer::LoadingMedia || status==QMediaPlayer::EndOfMedia) {
        player->stop();
        player->setPlaybackRate(-player->playbackRate());
        player->play();
    }
}

void VideoWidget::restartVideo(QMediaPlayer::MediaStatus status)
{
    if (status==QMediaPlayer::LoadingMedia || status==QMediaPlayer::EndOfMedia) {
        player->setPosition(0);
        player->play();
    }
}

/*
void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if ( event->button() == Qt::LeftButton ) {
        if ( player->state() == QMediaPlayer::PlayingState ) {
            player->pause();
            emit sendPause();
            emit sendPausePos(player->position());
        }
        else {
            player->bind(this);
            player->play();
            emit sendPlay();
        }
    }
    event->accept();
}

void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (view->cursor().shape() == Qt::ArrowCursor)
        view->setCursor(Qt::PointingHandCursor);
    event->accept();
}
*/


void VideoWidget::setGeometry(QRect const& rect)
{
    qDebug() << "Set geometry:" << rect;
    view->setGeometry(rect);
    scene->setSceneRect(rect);
    item->setOffset(rect.topLeft());
    item->setSize(rect.size());
}

void VideoWidget::setGeometry(int const x, int const y, int const w, int const h)
{
    qDebug() << "Set geometry:" << x << y << w << h;
    view->setGeometry(x, y, w, h);
    scene->setSceneRect(x, y, w, h);
    item->setOffset(QPointF(x,y));
    item->setSize(QSizeF(w,h));
}
