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

VideoWidget::VideoWidget(Poppler::MovieAnnotation const * annotation, QWidget* parent) : QVideoWidget(parent)
{
    this->annotation = annotation;
    setMouseTracking(true);
    player = new QMediaPlayer(this);
    player->setVideoOutput(this);
    Poppler::MovieObject * movie = annotation->movie();
    if (movie->showPosterImage()) {
        QPalette * palette = new QPalette();
        posterImage = movie->posterImage();
        if (!posterImage.isNull())
            palette->setBrush( QPalette::Window, QBrush(posterImage));
        setPalette(*palette);
        delete palette;
        setAutoFillBackground(true);
    }

    QUrl url = QUrl(movie->url());
    if (!url.isValid())
        QUrl url = QUrl::fromLocalFile(movie->url());
    if (url.isRelative())
        url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
    player->setMedia( url );
    switch (movie->playMode()) {
        case Poppler::MovieObject::PlayOpen:
            // TODO: PlayOpen should leave controls open (but they are not implemented yet).
        case Poppler::MovieObject::PlayOnce:
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::showPosterImage);
            break;
        case Poppler::MovieObject::PlayPalindrome:
            qWarning() << "WARNING: play mode=palindrome does not work as it should.";
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::bouncePalindromeVideo);
            break;
        case Poppler::MovieObject::PlayRepeat:
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::restartVideo);
            break;
    }
    if (movie->showControls()) {
        // TODO: video control bar
    }
    // Scale the video such that it fits the widget size.
    // I like these results, but I don't know whether this is what other people would expect.
    // Please write me a comment if you would prefer an other way of handling the aspect ratio.
    setAspectRatioMode(Qt::IgnoreAspectRatio);
    connect(player, &QMediaPlayer::positionChanged, this, &VideoWidget::positionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &VideoWidget::positionChanged);
}

VideoWidget::~VideoWidget()
{
    player->stop();
    player->disconnect();
    delete annotation;
    delete player;
}

QMediaPlayer const * VideoWidget::getPlayer() const
{
    return player;
}

qint64 VideoWidget::getDuration() const
{
    return player->duration();
}

void VideoWidget::setPosition(qint64 const position)
{
    player->setPosition(position);
}

Poppler::MovieAnnotation const * VideoWidget::getAnnotation() const
{
    return annotation;
}

void VideoWidget::play()
{
    if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
        player->bind(this);
    player->play();
}

void VideoWidget::pause()
{
    if (player->state() == QMediaPlayer::PlayingState)
        player->pause();
}

QMediaPlayer::State VideoWidget::state() const
{
    return player->state();
}

void VideoWidget::showPosterImage(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState && player->mediaStatus() == QMediaPlayer::EndOfMedia) {
        // Unbinding and binding this to the player is probably an ugly way of solving this.
        // TODO: find a better way of writing this.
        player->unbind(this);
        show();
    }
}

void VideoWidget::bouncePalindromeVideo(QMediaPlayer::State state)
{
    // TODO: The result of this function is not what it should be.
    // But this could also depend on the encoding of the video.
    if (state == QMediaPlayer::StoppedState) {
        disconnect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::bouncePalindromeVideo);
        player->stop();
        player->setPlaybackRate(-player->playbackRate());
        player->play();
        connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::bouncePalindromeVideo);
    }
}

void VideoWidget::restartVideo(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState) {
        if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
            player->setPosition(0);
        player->play();
    }
}

void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if ( event->button() == Qt::LeftButton ) {
        if ( player->state() == QMediaPlayer::PlayingState )
            player->pause();
        else {
            if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
                player->bind(this);
            player->play();
        }
    }
    event->accept();
}

void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (cursor() == Qt::ArrowCursor)
        setCursor(Qt::PointingHandCursor);
    event->accept();
}
