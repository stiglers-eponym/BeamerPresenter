// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef THUMBNAILBUTTON_H
#define THUMBNAILBUTTON_H

#include <QLabel>

#include "src/config.h"
#include "src/preferences.h"

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

  /// Sent current page to master, adjust style
  void sendPage()
  {
    setFocus();
    setStyleCurrent();
    emit sendNavigationSignal(page);
  }

  void giveFocusInner()
  {
    if (page == preferences()->page)
      setStyleCurrent();
    else
      setStyleFocus();
    emit updateFocus(this);
  }

  void setStyleDefault()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#00000000;color:#00000000;}");
  }

  void setStyleFocus()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#80ff0080;color:#80f00080;}");
  }

  void setStyleCurrent()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#ffff0000;color:#ffff0000;}");
  }

 public:
  static constexpr int line_width = 4;

  /// Boring constructor.
  ThumbnailButton(const int page, QWidget *parent = nullptr);

  void giveFocus()
  {
    setFocus();
    giveFocusInner();
  }

  void clearFocus()
  {
    if (page == preferences()->page)
      setStyleCurrent();
    else
      setStyleDefault();
  }

 protected:
  /// Mouse released: send navigation signal.
  void mouseReleaseEvent(QMouseEvent *event) override;
  /// Keyboard events for keyboard navigation
  void keyPressEvent(QKeyEvent *event) override;
  /// focus in: adjust/add margin
  void focusInEvent(QFocusEvent *) override { giveFocusInner(); }
  /// only implements workaround for allowing touchscreen scrolling
  bool event(QEvent *event) override;

 signals:
  /// Send out navigation event for this page.
  void sendNavigationSignal(int page);
  /// Tell thumbnail widget to set focus to this button.
  void updateFocus(ThumbnailButton *self);
  /// Tell thumbnail widget to move focus to the row above/below (updown=-1/+1).
  void focusUpDown(const char updown);
};

#endif  // THUMBNAILBUTTON_H
