// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/config.h"

#if (QT_VERSION_MAJOR < 6)
#include <QMediaPlaylist>
#endif

#include "src/media/mediaitem.h"
#include "src/media/mediaannotation.h"

MediaItem::MediaItem(std::shared_ptr<MediaAnnotation> &annotation) :
    _annotation(annotation)
{
#if (QT_VERSION_MAJOR >= 6)
    if (annotation->type() == MediaAnnotation::ExternalURL && annotation->flags() & MediaAnnotation::IsCaptureSession)
        _provider.reset(new MediaCaptureProvider());
    else
#endif
    {
        auto provider = new MediaPlayerProvider();
        provider->setMode(annotation->mode());
        _provider.reset(provider);
    }
    switch (annotation->type())
    {
    case MediaAnnotation::ExternalURL:
        _provider->setSource(std::static_pointer_cast<ExternalMedia>(annotation)->url());
        break;
    case MediaAnnotation::EmbeddedFile:
        _provider->setSourceDevice(&std::static_pointer_cast<EmbeddedMedia>(annotation)->buffer());
        break;
    case MediaAnnotation::EmbeddedAudioStream:
        break;
    }
}


std::shared_ptr<MediaItem> MediaItem::fromAnnotation(std::shared_ptr<MediaAnnotation> annotation, QGraphicsItem *parent)
{
    if (annotation == nullptr)
        return nullptr;
    if (annotation->flags() & MediaAnnotation::HasVideo)
        return std::shared_ptr<VideoItem>(new VideoItem(annotation, parent));
    else if (annotation->flags() & MediaAnnotation::HasAudio)
        return std::shared_ptr<AudioItem>(new AudioItem(annotation, parent));
    return nullptr;
}



AudioItem::AudioItem(std::shared_ptr<MediaAnnotation> &annotation, QGraphicsItem *parent) :
    QGraphicsRectItem(annotation->rect(), parent),
    MediaItem(annotation)
{}

VideoItem::VideoItem(std::shared_ptr<MediaAnnotation> &annotation, QGraphicsItem *parent) :
    QGraphicsVideoItem(parent),
    MediaItem(annotation)
{
    setPos(annotation->rect().topLeft());
    setSize(annotation->rect().size());
    _provider->setVideoOutput(this);
#if (QT_VERSION_MAJOR >= 6)
    _provider->setVideoSink(videoSink());
#endif
}

void MediaPlayerProvider::setMode(int mode) {
    switch (mode)
    {
    case MediaAnnotation::Once:
    case MediaAnnotation::Open:
        break;
    case MediaAnnotation::Palindrome:
        qWarning() << "Palindrome video: not implemented (yet)";
        // TODO
    case MediaAnnotation::Repeat:
    default:
#if (QT_VERSION_MAJOR >= 6)
        _player->setLoops(QMediaPlayer::Infinite);
        _player->connect(_player, &MediaPlayer::mediaStatusChanged, _player, &MediaPlayer::repeatIfFinished);
#else
        if (_player->playlist())
            _player->playlist()->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
#endif
        break;
    }
}

const QRectF &MediaItem::rect() const noexcept {return _annotation->rect();}
