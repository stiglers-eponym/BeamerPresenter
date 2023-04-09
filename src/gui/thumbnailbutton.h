// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef THUMBNAILBUTTON_H
#define THUMBNAILBUTTON_H

#include <QLabel>
#include "src/config.h"

class QMouseEvent;
class QKeyEvent;
class QFocusEvent;

/**
 * @brief Pushable button showing page preview
 *
 * @see ThumbnailWidget
 * @see ThumbnailThread
 */
class ThumbnailButton : public QLabel
{
    Q_OBJECT

    /// index of the page represented by this thumbnail
    const int page;

public:
    /// Boring constructor.
    ThumbnailButton(const int page, QWidget *parent = NULL);

protected:
    /// Mouse released: send navigation signal.
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// Keyboard events for keyboard navigation
    void keyPressEvent(QKeyEvent *event) override;
    /// add red margin when this gets focus
    void focusInEvent(QFocusEvent*) override;
    /// remove red margin when this looses focus
    void focusOutEvent(QFocusEvent*) override
    {setFrameStyle(QFrame::NoFrame);}

signals:
    /// Send out navigation event for this page.
    void sendNavigationSignal(int page);
};

#endif // THUMBNAILBUTTON_H
