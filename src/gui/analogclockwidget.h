// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ANALOGCLOCKWIDGET_H
#define ANALOGCLOCKWIDGET_H

#include <QWidget>
#include <QColor>
#include "src/config.h"
#include "src/enumerates.h"

class QTimer;
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

    /// Timer: regularly update clock
    QTimer *timer;
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

public:
    /// Constructor: create timer, connect to update event
    explicit AnalogClockWidget(QWidget *parent = nullptr);

    /// Destructor: delete timer
    ~AnalogClockWidget();

    /// Size hint: based on estimated size.
    QSize sizeHint() const noexcept override
    {return {60,60};}

    /// Height depends on width through font size.
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Read configuration from JSON object.
    void readConfig(const QJsonObject &config) noexcept;

protected:
    /// Event handler touch events.
    bool event(QEvent *event) override;

    /// Paint event: paint the clock
    void paintEvent(QPaintEvent*) override;

    /// Mouse double click starts/stops timer.
    void mouseDoubleClickEvent(QMouseEvent *event) noexcept override;

signals:
    /// Send action (toggle timer) to master.
    void sendAction(const Action);
};

#endif // ANALOGCLOCKWIDGET_H
