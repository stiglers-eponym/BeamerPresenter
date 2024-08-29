// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/thumbnailbutton.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QString>

ThumbnailButton::ThumbnailButton(const int page, QWidget *parent)
    : QLabel(parent), page(page)
{
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setToolTip(tr("page ") + QString::number(page + 1));
  setAttribute(Qt::WA_AcceptTouchEvents);
  QFrame::setFrameStyle(QFrame::Panel | QFrame::Plain);
  QFrame::setLineWidth(line_width);
  setStyleDefault();
}

void ThumbnailButton::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    emit updateFocus(this);
    sendPage();
    event->accept();
  }
}

void ThumbnailButton::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
      sendPage();
      event->accept();
      break;
    case Qt::Key_Left:
      focusPreviousChild();
      event->accept();
      break;
    case Qt::Key_Right:
      focusNextChild();
      event->accept();
      break;
    case Qt::Key_Up:
      emit focusUpDown(-1);
      event->accept();
      break;
    case Qt::Key_Down:
      emit focusUpDown(1);
      event->accept();
      break;
    default:
      event->ignore();
  }
}

bool ThumbnailButton::event(QEvent *event)
{
  // workaround for allowing touchscreen scrolling
  switch (event->type()) {
    case QEvent::TouchBegin:
      sendPage();
      event->accept();
      return true;
    default:
      return QLabel::event(event);
  }
}
