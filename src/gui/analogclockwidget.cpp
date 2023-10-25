// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QJsonObject>
#include <QJsonValue>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QTime>
#include <QTouchEvent>
#include <QtConfig>
#include <algorithm>
#if (QT_VERSION_MAJOR >= 6)
#include <QEventPoint>
#endif
#include "src/gui/analogclockwidget.h"
#include "src/log.h"

AnalogClockWidget::AnalogClockWidget(QWidget *parent) : QWidget{parent}
{
  setFocusPolicy(Qt::NoFocus);
  setMinimumSize(16, 16);
  setToolTip(tr("double-click on clock to start or pause timer"));
  timer_id = startTimer(1000);
}

void AnalogClockWidget::mouseDoubleClickEvent(QMouseEvent *event) noexcept
{
  emit sendAction(StartStopTimer);
  event->accept();
}

bool AnalogClockWidget::event(QEvent *event)
{
  switch (event->type()) {
    case QEvent::TouchBegin:
      event->accept();
      return true;
    case QEvent::TouchEnd: {
      event->accept();
      QTouchEvent *tevent = static_cast<QTouchEvent *>(event);
#if (QT_VERSION_MAJOR >= 6)
      if (tevent->pointCount() != 1) return false;
      debug_msg(DebugOtherInput, "Touch event on clock widget:" << tevent);
      const QEventPoint &point = tevent->point(0);
      // Short touch with a single point should start/stop the timer.
      if (point.position() == point.pressPosition() &&
          point.lastTimestamp() - point.pressTimestamp() < 100)
        emit sendAction(Action::StartStopTimer);
#else
      if (tevent->touchPoints().length() != 1) return false;
      debug_msg(DebugOtherInput, "Touch event on clock widget:" << tevent);
      const QTouchEvent::TouchPoint &point = tevent->touchPoints().first();
      // Touch event with a single point should start/stop the timer.
      if (point.pos() == point.startPos())
        emit sendAction(Action::StartStopTimer);
#endif
      return true;
    }
    default:
      return QWidget::event(event);
  }
}

void AnalogClockWidget::paintEvent(QPaintEvent *)
{
  const int side = std::min(width(), height());
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.translate(width() / 2., height() / 2.);
  painter.scale(0.45 * side, 0.45 * side);
  // Show ticks
  painter.setPen(QPen(tick_color, 0.05, Qt::SolidLine, Qt::RoundCap));
  for (int n = 0; n < 3; ++n) {
    painter.drawLine(QPointF(0.94, 0.), QPointF(1., 0.));
    painter.drawLine(QPointF(0., 0.94), QPointF(0., 1.));
    painter.drawLine(QPointF(-0.94, 0.), QPointF(-1., 0.));
    painter.drawLine(QPointF(0., -0.94), QPointF(0., -1.));
    painter.rotate(120);
  }
  if (small_ticks) {
    painter.setPen(QPen(tick_color, 0.02, Qt::SolidLine, Qt::RoundCap));
    for (int n = 0; n < 15; ++n) {
      if (n % 5 != 0) {
        painter.drawLine(QPointF(0.97, 0.), QPointF(1., 0.));
        painter.drawLine(QPointF(0., 0.97), QPointF(0., 1.));
        painter.drawLine(QPointF(-0.97, 0.), QPointF(-1., 0.));
        painter.drawLine(QPointF(0., -0.97), QPointF(0., -1.));
      }
      painter.rotate(24);
    }
  }
  const QTime time = QTime::currentTime();
  if (show_seconds) {
    // Show seconds
    painter.setPen(
        QPen(second_color, 0.02, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.save();
    if (timer_interval < 1000)
      painter.rotate(6 * time.second() + 0.006 * time.msec());
    else
      painter.rotate(6 * time.second());
    painter.drawLine(QPointF(0., 0.), QPointF(0., -0.93));
    painter.restore();
  }
  // Show minutes
  painter.setPen(
      QPen(minute_color, 0.08, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.save();
  painter.rotate(6 * time.minute() + 0.1 * time.second());
  painter.drawLine(QPointF(0., 0.), QPointF(0., -0.87));
  painter.restore();
  // Show hours
  painter.setPen(
      QPen(hour_color, 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter.save();
  painter.rotate(30 * time.hour() + 0.5 * time.minute());
  painter.drawLine(QPointF(0., 0.), QPointF(0., -0.7));
  painter.restore();
}

void AnalogClockWidget::readConfig(const QJsonObject &config) noexcept
{
  if (config.value("touch input").toBool(true))
    setAttribute(Qt::WA_AcceptTouchEvents);
  if (config.contains("seconds"))
    show_seconds = config.value("seconds").toBool(false);
  if (config.contains("aspect"))
    aspect_ratio =
        std::max(std::min(config.value("aspect").toDouble(1.), 10.), 0.1);
  if (config.contains("small ticks"))
    small_ticks = config.value("small ticks").toBool(false);
  if (config.contains("interval")) {
    killTimer(timer_id);
    timer_interval = std::min(config.value("interval").toInt(1000), 20);
    timer_id = startTimer(timer_interval);
  }
  if (config.contains("hour color"))
    hour_color = QColor(config.value("hour color").toString());
  if (config.contains("minute color"))
    minute_color = QColor(config.value("minute color").toString());
  if (config.contains("second color"))
    second_color = QColor(config.value("second color").toString());
  if (config.contains("tick color"))
    tick_color = QColor(config.value("tick color").toString());
}
