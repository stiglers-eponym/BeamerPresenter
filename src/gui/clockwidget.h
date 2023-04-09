// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QLineEdit>
#include <QTime>
#include "src/config.h"
#include "src/enumerates.h"

class QSize;
class QTimerEvent;
class QEvent;
class QResizeEvent;
class QMouseEvent;

/**
 * @brief Label showing the time.
 *
 * This is implemented as a QLineEdit because that one does not
 * request to update the layout when it's text is changed. There
 * is no need to recalculate the full layout every second because
 * the clock changes it's text.
 *
 * @see AnalogClockWidget
 * @see TimerWidget
 */
class ClockWidget : public QLineEdit
{
    Q_OBJECT

public:
    /// Constructor
    explicit ClockWidget(QWidget *parent = nullptr, bool accept_touch_input = true);

    /// Trivial destructor
    ~ClockWidget() {}

    /// Size hint: based on estimated size.
    QSize sizeHint() const noexcept override
    {return {125,20};}

    /// Height depends on width through font size.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Timeout event: update view.
    void timerEvent(QTimerEvent*) override
    {setText(QTime::currentTime().toString(Qt::TextDate));}

    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

    /// Event handler touch events.
    bool event(QEvent *event) override;

    /// Mouse double click starts/stops timer.
    void mouseDoubleClickEvent(QMouseEvent *event) noexcept override;

signals:
    /// Send action (toggle timer) to master.
    void sendAction(const Action);
};

#endif // CLOCKWIDGET_H
