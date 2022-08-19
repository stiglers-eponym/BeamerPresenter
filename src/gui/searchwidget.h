// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include "src/config.h"

class QLineEdit;
class QPushButton;

/**
 * @brief Widget for searching text in PDF
 * @todo shift search to separate thread
 * @todo better interface
 * @todo indicate failed search
 * @todo highlight search results
 */
class SearchWidget : public QWidget
{
    Q_OBJECT
    QLineEdit *search_field;
    QPushButton *forward_button;
    QPushButton *backward_button;

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget();
    /**
     * @brief search text of search_field and go to pages with matches
     * @param forward
     *   -1 for backward search,
     *   0 for forward search starting from current page,
     *   1 for forward search starting from next page
     */
    void search(qint8 forward = 0);

private slots:
    void searchCurrent() {search(0);}
    void searchForward() {search(1);}
    void searchBackward() {search(-1);}

signals:
    void foundPage(const int page);
};

#endif // SEARCHWIDGET_H
