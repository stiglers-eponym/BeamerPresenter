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
    playlist(new QMediaPlaylist(this)),
    annotation(annotation)
{
    // TODO: try connecting two views to the same graphics scene showing the video.
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("border: 0px");
    view->setAttribute(Qt::WA_TransparentForMouseEvents);
    view->setFocusPolicy(Qt::NoFocus);

    player->setVideoOutput(item);
    Poppler::MovieObject const *const movie = annotation->movie();
    if (movie->showPosterImage()) {
        posterImage = movie->posterImage();
        if (!posterImage.isNull()) {
            pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(posterImage));
            scene->addItem(pixmap);
        }
    }
    scene->addItem(item);

    filename = movie->url();
    url = QUrl(filename, QUrl::TolerantMode);
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
    if (!QFileInfo(url.fileName()).exists()) {
        filename = "";
        return;
    }
    playlist->addMedia(url);
    player->setPlaylist(playlist);
    if (splitFileName.contains("mute"))
        player->setMuted(true);
    if (splitFileName.contains("loop"))
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
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

    // Handle autostart arguments.
    // TODO: improve and document this.
    if (splitFileName.contains("noautostart") || splitFileName.contains("autostart=false"))
        autoplay = -1;
    else if (splitFileName.contains("autostart") || splitFileName.contains("autostart=true"))
        autoplay = 1;
}

VideoWidget::~VideoWidget()
{
    player->stop();
    player->disconnect();
    delete annotation;
    delete playlist;
    delete player;
    delete view;
    delete scene;
    // item is owned by scene.
}

void VideoWidget::play()
{
    item->show();
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
        scene->removeItem(item);
        hide();
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

void VideoWidget::setGeometry(QRect const& rect)
{
    view->setGeometry(rect);
    scene->setSceneRect(0,0,rect.width(), rect.height());
    item->setOffset({0.,0.});
    item->setSize(rect.size());
    scene->update(0,0,rect.width(), rect.height());
    if (!posterImage.isNull())
        pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(posterImage).scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void VideoWidget::setGeometry(int const x, int const y, int const w, int const h)
{
    view->setGeometry(x, y, w, h);
    scene->setSceneRect(0, 0, w, h);
    item->setOffset({0.,0.});
    item->setSize(QSizeF(w,h));
    scene->update(0, 0, w, h);
    if (!posterImage.isNull())
        pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(posterImage).scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
