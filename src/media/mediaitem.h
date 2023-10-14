// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <set>
#include <memory>
#include <utility>
#include <QGraphicsRectItem>
#include <QGraphicsVideoItem>

#include "src/config.h"
#include "src/log.h"
#include "src/enumerates.h"
#include "src/media/mediaprovider.h"
#include "src/media/mediaannotation.h"


class MediaPlayer;

/**
 * @brief MediaItem: abstract class for graphics items showing media
 *
 * Derived classes implement asQGraphicsItem() returning the item as
 * QGraphicsItem.
 */
class MediaItem
{
private:
    /// MediaAnnotation containing media information from the PDF
    std::shared_ptr<MediaAnnotation> _annotation = nullptr;

    /// Set of pages on which this video item appears. This is updated
    /// when videos for a new page are loaded and an old video is found
    /// to be visible also on the new page.
    std::set<int> _pages;

protected:
    /// Media provider providing the playback
    std::unique_ptr<MediaProvider> _provider = nullptr;

    /// Create a provider if it does not exist yet.
    void createProvider();

public:
    /// Construct item from annotation, creating a new provider
    MediaItem(std::shared_ptr<MediaAnnotation> &annotation, const int page) :
        _annotation(annotation)
    {
        debug_msg(DebugMedia, "creating media item" << this);
        _pages.insert(page);
        createProvider();
    }

    /// Trivial destructor
    virtual ~MediaItem() {};

    /// Create media provider if necessary
    virtual void initializeProvider()
    {createProvider();}

    /// Clear media provider to free memory
    void deleteProvider() {debug_msg(DebugMedia, "delete provider" << _provider.get()); _provider.reset(nullptr);};

    /// Read information from annotation and create a derived class
    /// of MediaItem using the given information.
    static std::shared_ptr<MediaItem> fromAnnotation(std::shared_ptr<MediaAnnotation> annotation, const int page, QGraphicsItem *parent=nullptr);

    /// annotation containing information fomr the PDF document
    const std::shared_ptr<MediaAnnotation> annotation() const noexcept
    {return _annotation;}

    /// annotation flags
    int flags() const noexcept
    {return _annotation->flags();}

    /// pages on which the video item appears
    const std::set<int> &pages() const noexcept
    {return _pages;}

    /// play media item
    void play() const
    {if (_provider) _provider->play();}

    /// pause media item
    void pause() const
    {if (_provider) _provider->pause();}

    /// toggle play/pause of media item. Return true of the playing state has actually changed.
    bool toggle() const
    {return _provider && _provider->toggle();}

    /* TODO: use idea behind old code for muting in SlideScene:
    const bool mute = (slide_flags & MuteSlide)
                      || (preferences()->global_flags & Preferences::MuteApplication)
                      || item->annotation()->volume() <= 0.
                      || (item->flags() & MediaItem::Mute);
     */
    /// mute or unmute this item
    void setMuted(const bool mute) const {if (_provider) _provider->setMuted(mute);}

    /// insert page into set of pages
    void insertPage(const int page) noexcept
    {_pages.insert(page);}

    /// check whether this has a playback provider, i.e. if something
    /// has been loaded and is probably ready for playing
    bool hasProvider() const noexcept {return _provider != nullptr;}

    /// show this object, shortcut for asGraphicsItem()->show()
    virtual void show() = 0;

    /// return this as QGraphicsItem*
    virtual QGraphicsItem *asQGraphicsItem() noexcept = 0;

    /// QGraphicsItem::type, shortcut for asQGraphicsItem()->type()
    virtual int type() const noexcept = 0;

    /// media annotation rectangle, taken from the geometry of the PDF annotation
    const QRectF &rect() const noexcept
    {return _annotation->rect();}

    /// check whether this item is currently playing
    bool isPlaying() const {return _provider && _provider->isPlaying();}

    /// media player, or nullptr if no media player is available
    MediaPlayer *player() const noexcept {
        if (_provider && _provider->type() == MediaProvider::PlayerType)
            return static_cast<MediaPlayerProvider*>(_provider.get())->player();
        return nullptr;
    }
};



/**
 * @brief AudioItem implementing MediaItem for audio-only media
 */
class AudioItem : public QGraphicsRectItem, public MediaItem
{
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + AudioItemType };

public:
    /// create AudioItem from PDF annotation
    AudioItem(std::shared_ptr<MediaAnnotation> &annotation, const int page, QGraphicsItem *parent=nullptr) :
        QGraphicsRectItem(annotation->rect(), parent),
        MediaItem(annotation, page)
    {}

    /// Trivial destructor
    virtual ~AudioItem() {}

    void show() override
    {QGraphicsRectItem::show();}

    QGraphicsItem *asQGraphicsItem() noexcept override
    {return this;}

    virtual int type() const noexcept override
    {return Type;}
};



/**
 * @brief VideoItem implementing MediaItem for media including video
 */
class VideoItem : public QGraphicsVideoItem, public MediaItem
{
    Q_OBJECT

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + VideoItemType };

    /// create VideoItem from PDF annotation
    VideoItem(std::shared_ptr<MediaAnnotation> &annotation, const int page, QGraphicsItem *parent=nullptr) :
        QGraphicsVideoItem(parent),
        MediaItem(annotation, page)
    {
        setPos(annotation->rect().topLeft());
        setSize(annotation->rect().size());
        _provider->setVideoOutput(this);
#if (QT_VERSION_MAJOR >= 6)
        _provider->setVideoSink(videoSink());
#endif
    }

    /// Trivial destructor
    virtual ~VideoItem() {}

    /// Create media provider if necessary
    void initializeProvider() override
    {
        if (_provider)
            return;
        createProvider();
        _provider->setVideoOutput(this);
#if (QT_VERSION_MAJOR >= 6)
        _provider->setVideoSink(videoSink());
#endif
    }

    void show() override
    {QGraphicsVideoItem::show();}

    QGraphicsItem *asQGraphicsItem() noexcept override
    {return this;}

    virtual int type() const noexcept override
    {return Type;}
};

#endif // MEDIAITEM_H
