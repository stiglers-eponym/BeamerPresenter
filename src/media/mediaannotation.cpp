// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/media/mediaannotation.h"

#include <QMap>
#include <QStringList>

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
