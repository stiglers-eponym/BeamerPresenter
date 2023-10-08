// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/config.h"

#if (QT_VERSION_MAJOR < 6)
#include <QMediaPlaylist>
#endif
#include <QBuffer>

#include "src/media/mediaitem.h"
#include "src/media/mediaannotation.h"

void MediaItem::createProvider()
{
    if (_provider)
        return;
#if (QT_VERSION_MAJOR >= 6)
    if (_annotation->type() == MediaAnnotation::ExternalURL && _annotation->flags() & MediaAnnotation::IsCaptureSession)
        _provider.reset(new MediaCaptureProvider());
    else
#endif
        _provider.reset(new MediaPlayerProvider());

    switch (_annotation->type())
    {
    case MediaAnnotation::ExternalURL:
        _provider->setSource(std::static_pointer_cast<ExternalMedia>(_annotation)->url());
        break;
    case MediaAnnotation::EmbeddedFile:
        _provider->setSourceData(std::static_pointer_cast<EmbeddedMedia>(_annotation)->data());
        break;
    case MediaAnnotation::EmbeddedAudioStream:
        break;
    }
    _provider->setMode(_annotation->mode());
}


std::shared_ptr<MediaItem> MediaItem::fromAnnotation(std::shared_ptr<MediaAnnotation> annotation, const int page, QGraphicsItem *parent)
{
    if (annotation == nullptr)
        return nullptr;
    if (annotation->flags() & MediaAnnotation::HasVideo)
        return std::shared_ptr<VideoItem>(new VideoItem(annotation, page, parent));
    else if (annotation->flags() & MediaAnnotation::HasAudio)
        return std::shared_ptr<AudioItem>(new AudioItem(annotation, page, parent));
    return nullptr;
}



void MediaPlayerProvider::setMode(const MediaAnnotation::Mode mode) {
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
