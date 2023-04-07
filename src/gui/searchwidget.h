// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>
#include "src/config.h"

class QFocusEvent;
class QPushButton;

/**
 * @brief Widget for searching text in PDF
 *
 * Currently, this searches for the first occurrence of a text on a page.
 * This first occurrence is highlighted. If the text is not found on the
 * current page, the following pages are searched. This nagivates to the
 * page on which the text is found.
 *
 * @todo shift search to separate thread
 * @todo better interface
 * @todo indicate failed search
 * @todo highlight all results on a page
 */
class SearchWidget : public QWidget
{
    Q_OBJECT

    /// text input widget
    QLineEdit *search_field;
    /// button for forward search
    QPushButton *forward_button;
    /// button for backward search
    QPushButton *backward_button;

protected:
    /// Focus event: focus search_field by default
    void focusInEvent(QFocusEvent*) override
    {search_field->setFocus();}

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();
    /**
     * @brief Search text of search_field and go to pages with matches.
     * Emits foundPages(page) once and returns when the text is found.
     * @param forward
     *   -1 for backward search,
     *   0 for forward search starting from current page,
     *   1 for forward search starting from next page
     */
    void search(qint8 forward = 0);

    /// Size hint: based on estimated size.
    QSize sizeHint() const noexcept override
    {return {180,18};}

    /// Height depends on width, this is required by the layout.
    bool hasHeightForWidth() const noexcept override
    {return true;}

private slots:
    /// Search on current page and following pages until text is found.
    void searchCurrent() {search(0);}
    /// Search starting on next page until text is found.
    void searchForward() {search(1);}
    /// Search backwards starting on previous page.
    void searchBackward() {search(-1);}

signals:
    /// Text has been found on given page.
    void searchPdf(const QString &text, const int page, const bool forward);
};

#endif // SEARCHWIDGET_H
