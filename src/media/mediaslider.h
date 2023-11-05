// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MEDIASLIDER_H
#define MEDIASLIDER_H

#include <QSlider>

#include "src/config.h"

/**
 * @brief extension of QSlider
 *
 * horizontal slider which accepts qint64 values for maximum and value.
 *
 * @see MediaPlayer
 */
class MediaSlider : public QSlider
{
  Q_OBJECT

 public:
  /// Constructor: disable focus.
  explicit MediaSlider(QWidget *parent = nullptr)
      : QSlider(Qt::Horizontal, parent)
  {
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QSlider::sliderPressed, this, &MediaSlider::press,
            Qt::DirectConnection);
  }

  /// Trivial destructor.
  ~MediaSlider() noexcept {}

 private slots:
  /// Called when slider is pressed to change position. This emitts
  /// a jumpTo signal with the current slider position.
  void press() { emit jumpTo(qint64(sliderPosition())); }

 public slots:
  /// Set maximum (time in ms).
  void setMaximumInt64(qint64 maximum) { setMaximum(int(maximum)); }

  /// Set position (time in ms).
  /* It can happen that a video has duration()==0, although the
   * video has a finite length. In this case the slider adjusts to
   * the video position it receives. */
  void setValueInt64(qint64 value)
  {
    if (value > maximum() + 250) setMaximum(value);
    setValue(int(value));
  }

 signals:
  /// jump to position, called after the slider was pressed (not moved).
  void jumpTo(qint64 position);
};

#endif  // MEDIASLIDER_H
