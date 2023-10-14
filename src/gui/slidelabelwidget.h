// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SLIDELABELWIDGET_H
#define SLIDELABELWIDGET_H

#include <QLineEdit>
#include <QSize>
#include "src/config.h"

class QLabel;
class QFocusEvent;
class QResizeEvent;

/**
 * @brief Widget showing current page label (editable) and last page label.
 *
 * Much of this is identical to SlideNumberWidget, but here the page
 * label is shown instead of the page index.
 *
 * @see SlideNumberWidget
 * @todo keyboard shortcuts while in QLineEdit.
 */
class SlideLabelWidget : public QWidget
{
    Q_OBJECT

    /// Show total number of pages here.
    QLabel *total;

    /// Show and edit current page number here.
    QLineEdit *edit;

public:
    /// Constructor: construct and connect everything.
    explicit SlideLabelWidget(QWidget *parent = nullptr);

    /// Trivial destructor: Qt should delete everything automatically.
    ~SlideLabelWidget() {}

    /// This should usually be a good estimate.
    QSize sizeHint() const noexcept override
    {return {150, 25};}

    /// Currently necessary because FlexLayout is kind of broken.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Focus event: focus text field by default
    void focusInEvent(QFocusEvent*) override
    {edit->setFocus(); edit->selectAll();}

    /// Resize: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

public slots:
    /// Update current slide and total number of slides.
    void updateText(const int page) noexcept;

    /// Read from edit and send navigation event.
    void readText() noexcept;

signals:
    /// Slide changed, send new page.
    void navigationSignal(const int page);
};

#endif // SLIDELABELWIDGET_H
