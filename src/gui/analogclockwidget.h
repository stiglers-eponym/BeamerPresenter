// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ANALOGCLOCKWIDGET_H
#define ANALOGCLOCKWIDGET_H

#include <QColor>
#include <QWidget>

#include "src/config.h"
#include "src/enumerates.h"

class QTimerEvent;
class QSize;
class QEvent;
class QMouseEvent;
class QPaintEvent;
class QJsonObject;

/**
 * @brief Widget showing an analog clock
 * @see ClockWidget
 */
class AnalogClockWidget : public QWidget
{
  Q_OBJECT

  static constexpr int double_click_ms = 100;
  static constexpr qreal minute_tick_start = 0.97;
  static constexpr qreal hour_tick_start = 0.94;
  static constexpr qreal second_hand_length = 0.93;
  static constexpr qreal minute_hand_length = 0.87;
  static constexpr qreal hour_hand_length = 0.7;
  static constexpr qreal second_hand_width = 0.02;
  static constexpr qreal minute_hand_width = 0.08;
  static constexpr qreal hour_hand_width = 0.1;
  static constexpr qreal minute_tick_width = 0.02;
  static constexpr qreal hour_tick_width = 0.05;

  /// interval (ms) for updating clock
  int timer_interval = 1000;
  /// timer id
  int timer_id;
  /// Enable/disable hand for seconds
  bool show_seconds = false;
  /// Enable/disable ticks every 6 degrees
  bool small_ticks = false;
  /// Color of hour hand
  QColor hour_color = Qt::gray;
  /// Color of minute hand
  QColor minute_color = Qt::black;
  /// Color of second hand
  QColor second_color = Qt::red;
  /// Color of ticks
  QColor tick_color = Qt::black;
  /// Preferred aspect ratio of the widget
  qreal aspect_ratio;

 public:
  /// Constructor: create timer, connect to update event
  explicit AnalogClockWidget(QWidget *parent = nullptr);
  /// Constructor: create timer, connect to update event, read config
  explicit AnalogClockWidget(const QJsonObject &config,
                             QWidget *parent = nullptr)
      : AnalogClockWidget(parent)
  {
    readConfig(config);
  }

  /// Trivial destructor
  ~AnalogClockWidget() {};

  /// Size hint: based on estimated size.
  QSize sizeHint() const noexcept override
  {
    return aspect_ratio > 1. ? QSize(aspect_ratio * 48, 48)
                             : QSize(48, 48 / aspect_ratio);
  }

  /// Height depends on width through font size.
  bool hasHeightForWidth() const noexcept override { return true; }

  /// Read configuration from JSON object.
  void readConfig(const QJsonObject &config) noexcept;

 protected:
  /// Timeout event: update view.
  void timerEvent(QTimerEvent *) override { update(); }

  /// Event handler touch events.
  bool event(QEvent *event) override;

  /// Paint event: paint the clock
  void paintEvent(QPaintEvent *) override;

  /// Mouse double click starts/stops timer.
  void mouseDoubleClickEvent(QMouseEvent *event) noexcept override;

 signals:
  /// Send action (toggle timer) to master.
  void sendAction(const Action);
};

#endif  // ANALOGCLOCKWIDGET_H
