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
    void focusInEvent(QFocusEvent*) override
    {
        setStyleSheet("ThumbnailButton{background-color:#ffff0000;color:#ffff0000;}");
        emit updateFocus(this);
    }

public slots:
    void defocus() noexcept
    {setStyleSheet("ThumbnailButton{background-color:#00000000;color:#00000000;}");}

signals:
    /// Send out navigation event for this page.
    void sendNavigationSignal(int page);
    /// Tell thumbnail widget to set focus to this button.
    void updateFocus(ThumbnailButton* self);
};

#endif // THUMBNAILBUTTON_H
