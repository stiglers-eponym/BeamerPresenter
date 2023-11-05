// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/pngpixmap.h"

#include <QBuffer>
#include <QByteArray>
#include <QPixmap>

#include "src/log.h"

PngPixmap::PngPixmap(const QPixmap pixmap, const int page,
                     const float resolution)
    : data(nullptr), resolution(resolution), page(page)
{
  // Check if the given pixmap is nontrivial
  if (pixmap.isNull() || pixmap.size().isEmpty() || pixmap.isDetached()) return;

  // Save the pixmap as PNG image.
  // First create a writable QByteArray and a QBuffer to write to it.
  QByteArray* bytes = new QByteArray();
  QBuffer buffer(bytes);
  // Save the pixmap as png to the buffer.
  const bool success =
      buffer.open(QIODevice::WriteOnly) && pixmap.save(&buffer, "PNG");
  buffer.close();
  if (success) {
    // Keep the result in data.
    data = bytes;
  } else {
    // saving failed, delete result.
    delete bytes;
    qWarning() << "Compressing image to PNG failed";
  }
}

const QPixmap PngPixmap::pixmap() const
{
  QPixmap pixmap;
  if (data == nullptr || data->isEmpty() || !pixmap.loadFromData(*data, "PNG"))
    qWarning() << "Loading image from PNG failed";
  return pixmap;
}
