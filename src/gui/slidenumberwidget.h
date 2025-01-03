// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDENUMBERWIDGET_H
#define SLIDENUMBERWIDGET_H

#include <QLineEdit>
#include <QSize>

#include "src/config.h"

class QLabel;
class QFocusEvent;
class QResizeEvent;

/**
 * @brief Widget showing current page number (editable) and total number of
 * pages.
 *
 * Much of this is identical to SlideLabelWidget, but here the page
 * index is shown instead of the page label.
 *
 * @see SlideLabelWidget
 * @todo keyboard shortcuts while in QLineEdit.
 */
class SlideNumberWidget : public QWidget
{
  Q_OBJECT

  /// Show total number of pages here.
  QLabel *total;

  /// Show and edit current page number here.
  QLineEdit *edit;

 public:
  /// Constructor: construct and connect everything.
  explicit SlideNumberWidget(QWidget *parent = nullptr);

  /// Trivial destructor: Qt should delete everything automatically.
  ~SlideNumberWidget() {}

  /// This should usually be a good estimate.
  QSize sizeHint() const noexcept override { return {150, 25}; }

  /// Currently necessary because FlexLayout is kind of broken.
  bool hasHeightForWidth() const noexcept override { return true; }

 protected:
  /// Focus event: focus text field by default
  void focusInEvent(QFocusEvent *) override
  {
    edit->setFocus();
    edit->selectAll();
  }

  /// Resize: adjust font size.
  void resizeEvent(QResizeEvent *event) noexcept override;

  /// Update current slide and total number of slides.
  void updateText(const int page) noexcept;

 public slots:
  /// Wrapper for updateText, ignores first argument
  void receivePage(const int slide, const int page) noexcept
  {
    updateText(page);
  }

  /// Read from edit and send navigation event.
  void readText() noexcept;

 signals:
  /// Slide changed, send new page.
  void sendPage(const int page);
};

#endif  // SLIDENUMBERWIDGET_H
