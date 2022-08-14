// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOCWIDGET_H
#define TOCWIDGET_H

#include <QScrollArea>
#include "src/gui/tocbutton.h"

class PdfDocument;

/**
 * @brief Widget showing document outline.
 *
 * The document outline is saved as a tree structure of TOCbuttons.
 * The tree root is first_button.
 *
 * @see TOCbutton
 */
class TOCwidget : public QScrollArea
{
    Q_OBJECT

    /// Root of TOCbutton tree representing the outline.
    TOCbutton *first_button = NULL;

public:
    /// Trivial constructor, does not create the outline tree.
    explicit TOCwidget(QWidget *parent = NULL) : QScrollArea(parent) {}

    /// Destructor: TOCbuttons are deleted recursively.
    ~TOCwidget()
    {delete first_button;}

    /// Generate the TOC from given document or preferences()->document.
    void generateTOC(const PdfDocument *document = NULL);

    /// Actually this is nonsense, but currently the layout only works with
    /// this option set.
    bool hasHeightForWidth() const override
    {return true;}

    /// Size hint required by layout.
    QSize sizeHint() const noexcept override
    {return {100, 200};}

public slots:
    /// Show event: generate outline if necessary. Expand to current position.
    void showEvent(QShowEvent*) override;

    /// Focus event: generate outline if necessary.
    void focusInEvent(QFocusEvent*) override
    {generateTOC();}

    /// Expand all sections, subsections, ... which contain the given page.
    void expandTo(const int page);

signals:
    /// Send navigation event to master.
    void sendNavigationSignal(const int page);
};

#endif // TOCWIDGET_H
