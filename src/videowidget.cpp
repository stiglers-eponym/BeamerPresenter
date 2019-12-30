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
    QVideoWidget(parent),
    player(new QMediaPlayer(this, QMediaPlayer::VideoSurface)),
    annotation(annotation)
{
    setMouseTracking(true);
    player->setVideoOutput(this);
    Poppler::MovieObject* movie = annotation->movie();
    if (movie->showPosterImage()) {
        QPalette* palette = new QPalette();
        posterImage = movie->posterImage();
        if (!posterImage.isNull())
            palette->setBrush( QPalette::Window, QBrush(posterImage));
        setPalette(*palette);
        delete palette;
        setAutoFillBackground(true);
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
    setAspectRatioMode(Qt::IgnoreAspectRatio);

    if (splitFileName.contains("autostart"))
        autoplay = true;
}

VideoWidget::~VideoWidget()
{
    player->stop();
    player->disconnect();
    delete annotation;
    delete player;
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
        show();
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
    if (cursor().shape() == Qt::ArrowCursor)
        setCursor(Qt::PointingHandCursor);
    event->accept();
}
