// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QLineEdit>
#include "src/config.h"
#include "src/enumerates_qt.h"

class QSize;
class QTimer;
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
 * @see TimerWidget
 */
class ClockWidget : public QLineEdit
{
    Q_OBJECT

    /// Timer: regularly update clock.
    QTimer *timer;

public:
    /// Constructor
    explicit ClockWidget(QWidget *parent = NULL, bool accept_touch_input = true);

    /// Destructor: delete timer.
    ~ClockWidget();

    /// Size hint: based on estimated size.
    QSize sizeHint() const noexcept override
    {return {125,20};}

    /// Height depends on width through font size.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

    /// Event handler touch events.
    bool event(QEvent *event) override;

    /// Mouse double click starts/stops timer.
    void mouseDoubleClickEvent(QMouseEvent *event) noexcept override;

private slots:
    /// Update label to show current time.
    void updateTime();

signals:
    /// Send action (toggle timer) to master.
    void sendAction(const Action);
};

#endif // CLOCKWIDGET_H
