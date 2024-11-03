// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/media/mediaprovider.h"

#include <QBuffer>
#include <QByteArray>
#include <memory>

#if (QT_VERSION_MAJOR >= 6)
#ifdef USE_WEBCAMS
#include <QCamera>
#include <QMediaDevices>
#endif  // USE_WEBCAMS
#else   // QT_VERSION_MAJOR
#include <QMediaPlaylist>
#endif  // QT_VERSION_MAJOR

#if (QT_VERSION_MAJOR < 6)
void MediaPlayerProvider::setSource(const QUrl &url)
{
  auto playlist = _player->playlist();
  if (!playlist)
    playlist = new QMediaPlaylist(_player);
  else
    playlist->clear();
  playlist->addMedia(url);
  _player->setPlaylist(playlist);
}

void MediaPlayerProvider::handleCommonError(QMediaPlayer::Error error)
{
  debug_msg(DebugMedia,
            "handling media error" << error << _player->errorString());
  if (error == QMediaPlayer::ResourceError && _buffer) {
    _buffer->seek(0);
    _player->setMedia(QMediaContent(), _buffer);
    _player->play();
  }
}
#endif  // QT_VERSION_MAJOR

void MediaPlayerProvider::setSourceData(std::shared_ptr<QByteArray> &data)
{
  debug_verbose(DebugMedia, "setting embedded media data" << this);
  _buffer = new QBuffer(data.get());
  _buffer->open(QBuffer::ReadOnly);
#if (QT_VERSION_MAJOR >= 6)
  _player->setSourceDevice(_buffer);
#else
  _player->setMedia(QMediaContent(), _buffer);
#endif
}

void MediaPlayerProvider::setMode(const MediaAnnotation::Mode mode)
{
  switch (mode) {
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
      QObject::connect(_player, &MediaPlayer::mediaStatusChanged, _player,
                       &MediaPlayer::repeatIfFinished);
#else
      if (_player->playlist())
        _player->playlist()->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
#endif
      break;
  }
}

#if (QT_VERSION_MAJOR >= 6) && defined(USE_WEBCAMS)
void MediaCaptureProvider::setSource(const QUrl &url)
{
  const auto inputs = QMediaDevices::videoInputs();
  for (const auto &cam : inputs) {
    if (cam.id() == url.path()) {
      QCamera *camera = new QCamera(cam);
      if (camera) {
        _session->setCamera(camera);
        camera->start();
      }
      return;
    }
  }
}
#endif
