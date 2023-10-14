// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/config.h"
#include "src/log.h"
#include "src/media/mediaitem.h"
#include "src/media/mediaannotation.h"


void MediaItem::createProvider()
{
    if (_provider)
        return;
#if (QT_VERSION_MAJOR >= 6) && defined(USE_WEBCAMS)
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
        qWarning() << "Embedded audio stream is currently not supported";
    default:
        return;
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
