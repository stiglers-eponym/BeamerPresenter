// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIAPROVIDER_H
#define MEDIAPROVIDER_H

#include <QBuffer>
#include <QGraphicsVideoItem>
#include <memory>

#include "src/config.h"
#if (QT_VERSION_MAJOR >= 6)
#include <QAudioOutput>
#include <QVideoSink>
#ifdef USE_WEBCAMS
#include <QCamera>
#include <QMediaCaptureSession>
#endif  // USE_WEBCAMS
#else   // QT_VERSION_MAJOR
#include <QMediaContent>
#endif  // QT_VERSION_MAJOR

#include "src/log.h"
#include "src/media/mediaannotation.h"
#include "src/media/mediaplayer.h"

/**
 * @brief MediaProvider abstract class providing multimedia functionality
 *
 * Derived classes can use a media player or a media capture session
 * to provide media content. The MediaProvider is used by a MediaItem
 * for playing media.
 */
class MediaProvider
{
 protected:
#if (QT_VERSION_MAJOR >= 6)
  /// Audio output
  QAudioOutput *audio_out = nullptr;
#endif

 public:
  /// subclass type
  enum Type {
    PlayerType,
#if (QT_VERSION_MAJOR >= 6) && defined(USE_WEBCAMS)
    CaptureType,
#endif
  };
  /// type used to distinguish derived classes
  virtual Type type() const noexcept = 0;

  /// constructor: creates audio output
  MediaProvider()
#if (QT_VERSION_MAJOR >= 6)
      : audio_out(new QAudioOutput())
#endif
  {
    debug_msg(DebugMedia, "creating media provider" << this);
  }

  /// destructor: deletes audio output
  virtual ~MediaProvider()
  {
#if (QT_VERSION_MAJOR >= 6)
    delete audio_out;
#endif
    debug_msg(DebugMedia, "deleting media provider" << this);
  }

  /// set media source from a URL
  virtual void setSource(const QUrl &url) = 0;

  /// set media source from (embedded) raw data
  virtual void setSourceData(std::shared_ptr<QByteArray> &data) {};

  /// set video output, see also setVideoSink
  virtual void setVideoOutput(QGraphicsVideoItem *out) = 0;

#if (QT_VERSION_MAJOR >= 6)
  /// set video sink (output), see also setVideoOutput
  virtual void setVideoSink(QVideoSink *sink) = 0;

  /// set audio output
  virtual void setAudioOutput(QAudioOutput *out) = 0;
#endif

  /// play media
  virtual void play() {};
  /// pause media
  virtual void pause() {};
  /// toggle play/pause
  virtual bool toggle() { return false; };

  /// check whether this is currently playing
  virtual bool isPlaying() const = 0;

  /// check whether this is muted
  virtual bool muted() const noexcept
#if (QT_VERSION_MAJOR >= 6)
  {
    return audio_out ? audio_out->isMuted() : true;
  }
#else
  {
    return false;
  }
#endif

  /// change playing mode
  virtual void setMode(const MediaAnnotation::Mode mode) {};

  /// mute or unmute
  virtual void setMuted(const bool mute) const
#if (QT_VERSION_MAJOR >= 6)
  {
    audio_out->setMuted(mute);
  }
#else
  {
  }
#endif

  /// change audio volume (number between 0 and 1)
  virtual void setVolume(const qreal volume) const
#if (QT_VERSION_MAJOR >= 6)
  {
    audio_out->setVolume(volume);
  }
#else
  {
  }
#endif

  /// clone function, apparently required for some unique_ptr operations?
  virtual std::unique_ptr<MediaProvider> clone() const = 0;
};

/**
 * @brief MediaPlayerProvider: Implements MediaProvider using a MediaPlayer
 *
 * This provider is used for all media types that acn be played using
 * QMediaPlayer, including embedded, local, and remote files, embedded
 * audio streams (not implemented yet!) and live streams.
 *
 * In Qt 5, this is implemented as QObject to receive signals for the
 * following reason: When playing media from a buffer, the media player
 * may run into a ressource error at the end of the file. This is
 * avoided by handling player error events here. If Qt 6, this does not
 * seem necessary.
 */
class MediaPlayerProvider :
#if (QT_VERSION_MAJOR < 6)
    public QObject,
#endif
    public MediaProvider
{
#if (QT_VERSION_MAJOR < 6)
  Q_OBJECT
#endif

  /// Media player, owned by this. _player should never be nullptr.
  MediaPlayer *_player = nullptr;

  /// Buffer providing media data, owned by this. A buffer is only
  /// created when playing an embedded file. Thus this will often
  /// be nullptr.
  QBuffer *_buffer = nullptr;

 private slots:
#if (QT_VERSION_MAJOR < 6)
  /**
   * Handle resource errors for media played from buffer by reconnecting
   * to the buffer. This aims at handling an error that occurs when using
   * Qt 5 and playing an embedded video file.
   */
  void handleCommonError(QMediaPlayer::Error error);
#endif  // QT_VERSION_MAJOR

 public:
  /// Basic constructor: creates media player
#if (QT_VERSION_MAJOR >= 6)
  MediaPlayerProvider() : MediaProvider(), _player(new MediaPlayer())
  {
    _player->setAudioOutput(audio_out);
  }
#else  // QT_VERSION_MAJOR
  MediaPlayerProvider() : QObject(), MediaProvider(), _player(new MediaPlayer())
  {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    _player->setAudioRole(QAudio::VideoRole);
#endif  // QT_VERSION >= 5.6
    connect(_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &MediaPlayerProvider::handleCommonError);
  }
#endif  // QT_VERSION_MAJOR

  /// Constructor: Takes ownership of given player.
  /// If player is nullptr, a new MediaPlayer object is created.
#if (QT_VERSION_MAJOR >= 6)
  MediaPlayerProvider(MediaPlayer *player, QObject *parent = nullptr)
      : MediaProvider(), _player(player)
  {
    if (!_player) _player = new MediaPlayer();
    if (_player) _player->setAudioOutput(audio_out);
  }
#else   // QT_VERSION_MAJOR
  MediaPlayerProvider(MediaPlayer *player, QObject *parent = nullptr)
      : QObject(parent), MediaProvider(), _player(player)
  {
    if (!_player) _player = new MediaPlayer();
    connect(_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &MediaPlayerProvider::handleCommonError);
  }
#endif  // QT_VERSION_MAJOR

#if (QT_VERSION_MAJOR >= 6)
  MediaPlayerProvider(const MediaPlayerProvider &other)
      : MediaProvider(), _player(other._player)
  {
    if (_player) _player->setAudioOutput(audio_out);
  }
#else   // QT_VERSION_MAJOR
  MediaPlayerProvider(const MediaPlayerProvider &other)
      : QObject(other.parent()), MediaProvider(), _player(other._player)
  {
    connect(_player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &MediaPlayerProvider::handleCommonError);
  }
#endif  // QT_VERSION_MAJOR

  /// Destructor: delete _player and _buffer
  ~MediaPlayerProvider()
  {
    delete _player;
    delete _buffer;
  }

  Type type() const noexcept override { return PlayerType; }

#if (QT_VERSION_MAJOR >= 6)
  void setSource(const QUrl &url) override { _player->setSource(url); }
#else
  void setSource(const QUrl &url) override;
#endif

  void setSourceData(std::shared_ptr<QByteArray> &data) override;

  void setVideoOutput(QGraphicsVideoItem *out) override
  {
    _player->setVideoOutput(out);
  }

#if (QT_VERSION_MAJOR >= 6)
  void setAudioOutput(QAudioOutput *out) override
  {
    _player->setAudioOutput(out);
  }

  void setVideoSink(QVideoSink *sink) override { _player->setVideoSink(sink); }
#endif  // QT_VERSION_MAJOR

  void play() override { _player->play(); }

  void pause() override { _player->pause(); }

  bool toggle() override
  {
    debug_msg(DebugMedia,
              "toggle play:" << _player->isPlaying() << _player->mediaStatus());
    if (_player->isPlaying())
      pause();
    else
      play();
    return true;
  }

  bool isPlaying() const override { return _player->isPlaying(); }

  void setMode(const MediaAnnotation::Mode mode) override;

  /// get media player
  MediaPlayer *player() const noexcept { return _player; }

  virtual std::unique_ptr<MediaProvider> clone() const override
  {
    return std::make_unique<MediaPlayerProvider>(*this);
  }

#if (QT_VERSION_MAJOR < 6)
  /// check whether this is muted
  bool muted() const noexcept override
  {
    return _player ? _player->isMuted() : true;
  }

  /// mute or unmute
  void setMuted(const bool mute) const override
  {
    if (_player) _player->setMuted(mute);
  }

  /// set audio volume
  void setVolume(const qreal volume) const override
  {
    if (_player) _player->setVolume(100 * volume);
  }
#endif  // QT_VERSION_MAJOR
};

#if (QT_VERSION_MAJOR >= 6) && defined(USE_WEBCAMS)

/**
 * @brief MediaCaptureProvider: Implements MediaProvider using a media capture
 * session
 *
 * This MediaProvider uses the camera provided by a
 * QMediaCaptureSession to show the output of a webcam as a video
 * annotation.
 */
class MediaCaptureProvider : public MediaProvider
{
  /// media session providing the webcam, owned by this and should
  /// never be nullptr
  QMediaCaptureSession *_session = nullptr;

 public:
  /// Constructor: create new empty media capture session
  MediaCaptureProvider() : MediaProvider(), _session(new QMediaCaptureSession())
  {
    _session->setAudioOutput(audio_out);
  }

  /// Constructor: takes ownership of given session. If session is
  /// nullptr, a new media session is created.
  MediaCaptureProvider(QMediaCaptureSession *session)
      : MediaProvider(), _session(session)
  {
    if (!_session) _session = new QMediaCaptureSession();
    if (_session) _session->setAudioOutput(audio_out);
  }

  /// Destructor: delete camera and session
  ~MediaCaptureProvider()
  {
    delete _session->camera();
    delete _session;
  }

  Type type() const noexcept override { return CaptureType; }

  bool isPlaying() const override { return _session->camera(); }

  void setAudioOutput(QAudioOutput *out) override
  {
    _session->setAudioOutput(out);
  }

  void setVideoOutput(QGraphicsVideoItem *out) override
  {
    _session->setVideoOutput(out);
  }

  void setVideoSink(QVideoSink *sink) override { _session->setVideoSink(sink); }

  void setSource(const QUrl &url) override;

  virtual std::unique_ptr<MediaProvider> clone() const override
  {
    return std::make_unique<MediaCaptureProvider>(*this);
  }
};

#endif  // USE_WEBCAMS and QT_VERSION_MAJOR

#endif  // MEDIAPROVIDER_H
