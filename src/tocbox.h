/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TOCBOX_H
#define TOCBOX_H

#include <QWidget>
#include <QMenu>
#include <QVBoxLayout>
#include <QDomDocument>
#include <QGuiApplication>
#include <QComboBox>
#include "pdfdoc.h"
#include "tocbutton.h"
#include "tocaction.h"

/// Strings indicating the different levels of different TOC entries.
static QStringList const tocIndentStrings = {"  ", "    ➤ ", "       - ", "          + "};

/// Widget showing the table of contents.
/// This widget is shown on top of the notes widget.
class TocBox : public QWidget
{
    Q_OBJECT

private:
    /// table of contents as returned by poppler.
    QDomDocument const* toc = nullptr;
    /// Maximum level of TOC entries shown directly. One further level is shown in a drop down menu.
    quint8 unfoldLevel = 2;
    /// Layout.
    QVBoxLayout* layout;
    /// List of buttons (corresponding to TOC entries).
    QList<TocButton*> buttons;
    /// List of drop down menus used to show entries one level deeper than unfoldLevel.
    QList<QMenu*> menus;
    /// Map pages to button indices. This can be used to select the button for the current page.
    QMap<int, int> page_to_button;

    /// Recursively create the table of contents.
    void recursiveTocCreator(QDomNode const& node, quint8 const level);

    /// Indicates whether the TOC GUI should be updated before it is shown.
    /// Initially the GUI is not initialized and therefore needs an update.
    bool need_update = true;

    /// PDF document (presentation). This is used to convert destination strings to page indices.
    PdfDoc const* pdf = nullptr;

public:
    // Constructor, destructor.
    TocBox(QWidget* parent = nullptr);
    ~TocBox();
    // Create or update GUI. Return true if an error occured or no TOC GUI was created.
    bool createToc();
    // Set the unfold level.
    void setUnfoldLevel(quint8 const level);
    // Set the document (called only once).
    void setPdf(PdfDoc const* doc) {pdf=doc;}
    // Mark TOC GUI as outdated.
    void setOutdated() {need_update = true;}
    // Check whether a TOC has been created.
    bool hasToc() {return toc!=nullptr;}
    // Focus on current page.
    void focusCurrent(int const page);

signals:
    /// Send a new page number. This shows the new page on the presentation screen.
    void sendNewPage(int const page);
};

#endif // TOCBOX_H
