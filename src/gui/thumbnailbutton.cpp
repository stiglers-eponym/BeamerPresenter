// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QMouseEvent>
#include <QKeyEvent>
#include <QPalette>
#include <QString>
#include "src/gui/thumbnailbutton.h"

ThumbnailButton::ThumbnailButton(const int page, QWidget *parent) :
    QLabel(parent),
    page(page)
{
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setToolTip(tr("page ") + QString::number(page+1));
}


void ThumbnailButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit sendNavigationSignal(page);
        event->accept();
    }
}

void ThumbnailButton::focusInEvent(QFocusEvent*)
{
    setFrameStyle(QFrame::Panel);
    setLineWidth(2);
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);
    setPalette(palette);
}


void ThumbnailButton::focusOutEvent(QFocusEvent*)
{
    setFrameStyle(QFrame::NoFrame);
}

void ThumbnailButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        emit sendNavigationSignal(page);
        event->accept();
        break;
    case Qt::Key_Left:
        focusPreviousChild();
        break;
    case Qt::Key_Right:
        focusNextChild();
        break;
    case Qt::Key_Escape:
        clearFocus();
        break;
    default:
        event->ignore();
    }
}
