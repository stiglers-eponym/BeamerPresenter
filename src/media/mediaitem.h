// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include <set>
#include <memory>
#include <utility>
#include "src/config.h"
#include <QIODevice>
#include <QGraphicsRectItem>
#include <QGraphicsVideoItem>

#if (QT_VERSION_MAJOR >= 6)
#include <QVideoSink>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QCamera>
#include <QMediaCaptureSession>
#else
#include <QMediaContent>
#endif

#include "src/log.h"
#include "src/media/mediaplayer.h"

class MediaAnnotation;


/**
 * @brief MediaProvider abstract class providing multimedia functionality
 *
 * Derived classes can use a media player or a media capture session
 * to provide media content.
 */
class MediaProvider
{
protected:
#if (QT_VERSION_MAJOR >= 6)
    /// Audio output
    QAudioOutput *audio_out = nullptr;
#endif

public:
    enum Type {
        PlayerType,
        CaptureType,
    };
    /// type used to distinguish derived classes
    virtual Type type() const noexcept = 0;

    /// constructor: creates audio output
#if (QT_VERSION_MAJOR >= 6)
    MediaProvider() : audio_out(new QAudioOutput()) {}
#else
    MediaProvider() {}
#endif

    /// destructor: deletes audio output
#if (QT_VERSION_MAJOR >= 6)
    virtual ~MediaProvider() {delete audio_out;}
#else
    virtual ~MediaProvider() {}
#endif

    /// set media source from a URL
    virtual void setSource(const QUrl &url) = 0;

    /// set media source from a bit stream provided as QIODevice
    virtual void setSourceDevice(QIODevice *device) {};

    /// set video output (QObject), see also setVideoSink
    virtual void setVideoOutput(QObject* out) = 0;

#if (QT_VERSION_MAJOR >= 6)
    /// set video sink (output), see also setVideoOutput
    virtual void setVideoSink(QVideoSink* sink) = 0;

    /// set audio output
    virtual void setAudioOutput(QAudioOutput* out) = 0;
#endif

    /// play media
    virtual void play() {};
    /// pause media
    virtual void pause() {};
    /// toggle play/pause
    virtual bool toggle() {return false;};
    /// check whether this is currently playing
    virtual bool isPlaying() const = 0;

    /// check whether this is muted
    virtual bool muted() const noexcept
#if (QT_VERSION_MAJOR >= 6)
    {return audio_out ? audio_out->isMuted() : true;}
#else
    {return false;}
#endif

    /// mute or unmute
    virtual void setMuted(const bool mute) const
#if (QT_VERSION_MAJOR >= 6)
    {audio_out->setMuted(mute);}
#else
    {}
#endif

    /// clone function, apparently required for some unique_ptr operations?
    virtual std::unique_ptr<MediaProvider> clone() const = 0;
};



/**
 * @brief MediaPlayerProvider: Implements MediaProvider using a MediaPlayer
 */
class MediaPlayerProvider : public MediaProvider
{
    MediaPlayer *_player = nullptr;

public:
    MediaPlayerProvider() : _player(new MediaPlayer())
    {
#if (QT_VERSION_MAJOR >= 6)
        _player->setAudioOutput(audio_out);
#endif
    }

    MediaPlayerProvider(MediaPlayer *player) : _player(player)
    {
#if (QT_VERSION_MAJOR >= 6)
        _player->setAudioOutput(audio_out);
#endif
    }

    ~MediaPlayerProvider() {delete _player;}

    Type type() const noexcept override
    {return PlayerType;}

    void setSource(const QUrl &url) override
#if (QT_VERSION_MAJOR >= 6)
    {_player->setSource(url);}
#else
    {_player->setMedia(QMediaContent(url));}
#endif

    void setSourceDevice(QIODevice *device) override
#if (QT_VERSION_MAJOR >= 6)
    {_player->setSourceDevice(device);}
#else
    {_player->setMedia(QMediaContent(), device);}
#endif

    void setVideoOutput(QObject* out) override
#if (QT_VERSION_MAJOR >= 6)
    {_player->setVideoOutput(out);}
#else
    {_player->setVideoOutput(dynamic_cast<QGraphicsVideoItem*>(out));}
#endif

#if (QT_VERSION_MAJOR >= 6)
    void setAudioOutput(QAudioOutput* out) override
    {_player->setAudioOutput(out);}

    void setVideoSink(QVideoSink* sink) override
    {_player->setVideoSink(sink);}
#endif

    void play() override
    {_player->play();}

    void pause() override
    {_player->pause();}

    bool toggle() override
    {
        debug_msg(DebugMedia, "toggle play:" << _player->isPlaying());
        if (_player->isPlaying())
            _player->pause();
        else
            _player->play();
        return true;
    }

    bool isPlaying() const override
    {return _player->isPlaying();}

    void setMode(int mode);

    MediaPlayer *player() const noexcept
    {return _player;}

    virtual std::unique_ptr<MediaProvider> clone() const override
    {return std::make_unique<MediaPlayerProvider>(*this);}

    /// mute or unmute
#if (QT_VERSION_MAJOR < 6)
    bool muted() const noexcept override
    {return _player ? _player->isMuted() : true;}

    void setMuted(const bool mute) const override
    {if (_player) _player->setMuted(mute);}
#endif
};



#if (QT_VERSION_MAJOR >= 6)
/**
 * @brief MediaCaptureProvider: Implements MediaProvider using a media capture session
 */
class MediaCaptureProvider : public MediaProvider
{
    QMediaCaptureSession *session = nullptr;

public:
    MediaCaptureProvider() : session(new QMediaCaptureSession())
    {session->setAudioOutput(audio_out);}

    MediaCaptureProvider(QMediaCaptureSession *session) : session(session)
    {session->setAudioOutput(audio_out);}

    ~MediaCaptureProvider()
    {
        if (session)
            delete session->camera();
        delete session;
    }

    Type type() const noexcept override {return PlayerType;}

    bool isPlaying() const override
    {return session && session->camera();}

    void setAudioOutput(QAudioOutput* out) override
    {session->setAudioOutput(out);}

    void setVideoOutput(QObject* out) override
    {session->setVideoOutput(out);}

    void setVideoSink(QVideoSink* sink) override
    {session->setVideoSink(sink);}

    void setSource(const QUrl &url) override {
        for (const auto &cam : QMediaDevices::videoInputs())
        {
            if (cam.id() == url.path())
            {
                QCamera *camera = new QCamera(cam);
                if (camera)
                {
                    session->setCamera(new QCamera(cam));
                    camera->start();
                }
                return;
            }
        }
    }

    virtual std::unique_ptr<MediaProvider> clone() const override
    {return std::make_unique<MediaCaptureProvider>(*this);}
};
#endif



/**
 * @brief MediaItem: abstract class for graphics items showing media
 *
 * Derived classes implement asQGraphicsItem() returning the item as
 * QGraphicsItem.
 */
class MediaItem
{
public:
    enum Flags
    {
        IsLive = 0x04, ///< shows a live video
        Autoplay = 0x08, ///< autoplay video
        ShowSlider = 0x10, ///< show slider on control screen
        AllowPause = 0x20, ///< allow pausing the video
        Mute = 0x40, ///< mute audio
    };

private:
    /// MediaAnnotation containing media information from the PDF
    std::shared_ptr<MediaAnnotation> _annotation = nullptr;

    /// Flags... TODO!
    int _flags = Autoplay|ShowSlider|AllowPause;

    /// Set of pages on which this video item appears. This is updated
    /// when videos for a new page are loaded and an old video is found
    /// to be visible also on the new page.
    std::set<int> _pages;

protected:
    /// Media provider providing the playback
    std::unique_ptr<MediaProvider> _provider = nullptr;

public:
    /// Construct item from annotation, creating a new provider
    MediaItem(std::shared_ptr<MediaAnnotation> &annotation);

    virtual ~MediaItem() {};

    /// Read information from annotation and create a derived class
    /// of MediaItem using the given information.
    static std::shared_ptr<MediaItem> fromAnnotation(std::shared_ptr<MediaAnnotation> annotation, QGraphicsItem *parent=nullptr);

    /// annotation containing information fomr the PDF document
    const std::shared_ptr<MediaAnnotation> annotation() const noexcept
    {return _annotation;}

    /// Flags... TODO!
    int flags() const noexcept
    {return _flags;}

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
    const QRectF &rect() const noexcept;

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
public:
    /// create AudioItem from PDF annotation
    AudioItem(std::shared_ptr<MediaAnnotation> &annotation, QGraphicsItem *parent=nullptr);
    virtual ~AudioItem() {}

    void show() override
    {QGraphicsRectItem::show();}

    QGraphicsItem *asQGraphicsItem() noexcept override
    {return this;}

    virtual int type() const noexcept override
    {return QGraphicsItem::UserType + 20;}
};



/**
 * @brief VideoItem implementing MediaItem for media including video
 */
class VideoItem : public QGraphicsVideoItem, public MediaItem
{
    Q_OBJECT

public:
    /// create VideoItem from PDF annotation
    VideoItem(std::shared_ptr<MediaAnnotation> &annotation, QGraphicsItem *parent=nullptr);
    virtual ~VideoItem() {}

    void show() override
    {QGraphicsVideoItem::show();}

    QGraphicsItem *asQGraphicsItem() noexcept override
    {return this;}

    virtual int type() const noexcept override
    {return QGraphicsItem::UserType + 21;}
};

#endif // MEDIAITEM_H
