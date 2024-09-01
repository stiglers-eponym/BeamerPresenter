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

  /// Adjust style after giving focus to this button.
  /// Notify parent that focus has changed.
  void giveFocusInner()
  {
    if (page == preferences()->page)
      setStyleCurrent();
    else
      setStyleFocus();
    emit updateFocus(this);
  }

  /// Set style to default (unfocussed, not current page)
  void setStyleDefault()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#00000000;color:#00000000;}");
  }

  /// Set style to focussed but not the current page.
  void setStyleFocus()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#80ff0080;color:#80f00080;}");
  }

  /// Set style to current page.
  void setStyleCurrent()
  {
    setStyleSheet(
        "ThumbnailButton{background-color:#ffff0000;color:#ffff0000;}");
  }

 public:
  /// Margin line width.
  static constexpr int line_width = 4;

  /// Constructor: prepare style.
  ThumbnailButton(const int page, QWidget *parent = nullptr);

  /// Set focus and explicitly set layout.
  /// Make sure that giveFocusInner() is called even if focus is not set.
  void giveFocus()
  {
    setFocus();
    if (!hasFocus()) giveFocusInner();
  }

  /// Adjust style for unfocussed button.
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
