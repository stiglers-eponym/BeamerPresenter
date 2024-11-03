// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/media/mediaannotation.h"

#include <QMap>
#include <QStringList>
#include <algorithm>

ExternalMedia::ExternalMedia(const QUrl &url, const QRectF &rect,
                             const Mode mode, const MediaFlags flags)
    : MediaAnnotation(rect, mode, flags), _url(url)
{
  const QString scheme = url.scheme();
  if (scheme == "v4l" || scheme == "v4l2" || scheme == "cam")
    _flags |= IsLive | IsCaptureSession;
  else if (scheme == "udp" || scheme == "rtp")
    _flags |= IsLive;
  if (url.hasQuery()) {
    static const QMap<QString, MediaFlag> flag_names{
        {"live", IsLive}, {"autoplay", Autoplay},       {"slider", ShowSlider},
        {"mute", Mute},   {"interaction", Interactive},
    };
    QStringList query = url.query().toLower().split("&");
    for (auto it = query.begin(); it != query.end();) {
      const QStringList key_value = it->split("=");
      if (key_value.length() == 2) {
        const MediaFlag target =
            flag_names.value(key_value.first(), InvalidFlag);
        if (target != InvalidFlag) {
          if (key_value.last() == "true")
            _flags |= target;
          else if (key_value.last() == "false")
            _flags &= ~target;
          else {
            ++it;
            continue;
          }
          it = query.erase(it);
          continue;
        }
      }
      ++it;
    }
    _url.setQuery(query.join("&"));
  }
}

bool ExternalMedia::operator==(const MediaAnnotation &other) const noexcept
{
  if (type() != other.type() || mode() != other.mode() ||
      rect().toAlignedRect() != other.rect().toAlignedRect())
    return false;
  const auto &other_ext = static_cast<const ExternalMedia &>(other);
  return _url == other_ext._url;
}

bool EmbeddedMedia::operator==(const MediaAnnotation &other) const noexcept
{
  if (type() != other.type() || mode() != other.mode() ||
      rect().toAlignedRect() != other.rect().toAlignedRect())
    return false;
  const auto &other_em = static_cast<const EmbeddedMedia &>(other);
#if (QT_VERSION_MAJOR >= 6)
  return _data->size() == other_em._data->size() &&
         other_em._data->startsWith(
             _data->first(std::min(_data->size(), qsizetype(64))));
#else
  return _data->size() == other_em._data->size();
#endif
}

bool EmbeddedAudio::operator==(const MediaAnnotation &other) const noexcept
{
  if (type() != other.type() || mode() != other.mode() ||
      rect().toAlignedRect() != other.rect().toAlignedRect())
    return false;
  const auto &other_em = static_cast<const EmbeddedAudio &>(other);
  return _data->size() == other_em._data->size()
#if (QT_VERSION_MAJOR >= 6)
         && other_em._data->startsWith(
                _data->first(std::min(_data->size(), qsizetype(64))))
#endif
         && sampling_rate == other_em.sampling_rate &&
         encoding == other_em.encoding && channels == other_em.channels &&
         bits_per_sample == other_em.bits_per_sample;
}
