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

#include "tocbox.h"

/// Basic constructor: create layout, but do not construct the TOC yet.
TocBox::TocBox(QWidget* parent) :
    QWidget(parent),
    layout(new QVBoxLayout(this))
{
    // The empty layout will be filled with buttons later.
    // Use the layout.
    this->setLayout(layout);
}

/// Create or update the TOC box if it needs an update.
/// This creates the buttons and drop down menus.
/// Return true if no TOC GUI was created.
bool TocBox::createToc()
{
    // Check whether an update is needed.
    if (!need_update)
        return toc==nullptr || toc->isNull();
    // Get the (new) table of contents.
    QDomDocument const* newToc = pdf->getToc();
    // Check whether an update is needed (and provided).
    if (!need_update || newToc==nullptr || newToc->isNull())
        return toc==nullptr || toc->isNull();
    // Clean the existing TOC buttons if necessary.
    if (toc != nullptr) {
        // Clear all menus.
        for (QList<QMenu*>::const_iterator menu=menus.cbegin(); menu!=menus.cend(); menu++)
            qDeleteAll((*menu)->actions());
        // Delete all menus.
        qDeleteAll(menus);
        menus.clear();
        // Delete all buttons.
        qDeleteAll(buttons);
        buttons.clear();
        // Delete the old TOC document.
        delete toc;
    }
    // Update toc.
    toc = newToc;
    // Read the QDomDocument.
    // Iterate through all nodes in toc.
    for(QDomNode n=toc->firstChild(); !n.isNull(); n=n.nextSibling()) {
        // Create the TOC button for this button and buttons for all subitems of this TOC entry.
        recursiveTocCreator(n, 0);
    }
    // TOC has been updated. No further updates required.
    need_update = false;
    return buttons.length()==0;
}

/// Set the maximum number of unfolded TOC levels.
void TocBox::setUnfoldLevel(quint8 const level)
{
    if (unfoldLevel != level) {
        // After changing the unfold level, the TOC buttons need to be regenerated.
        need_update = true;
        unfoldLevel = level;
    }
}

/// Create the TOC buttons and subbuttons of a given TOC node recursively.
/// This takes a QDomNode and the level as arguments.
/// The recursion is limited to maximum unfoldLevel iterations.
void TocBox::recursiveTocCreator(QDomNode const& n, quint8 const level)
{
    /// Element of the current TOC button.
    QDomElement const e = n.toElement();
    // Check whether this element is nontrivial
    if (e.isNull())
        return;
    // Get the destination page of this TOC item.
    int const dest = pdf->destToSlide(e.attribute("DestinationName", ""));
    // Create the button representing this TOC item in the GUI.
    TocButton* button = new TocButton(tocIndentStrings[level], e.tagName(), dest, this);
    // Hand over the event if the button is pushed.
    connect(button, &TocButton::activated, this, &TocBox::sendNewPage);
    // Save the mapping of page destination to button index.
    // buttons.length() is the index of the current button.
    page_to_button[dest] = buttons.length();
    // Add the button to the list of buttons.
    buttons.append(button);
    // Add the button to the layout.
    layout->addWidget(button);
    // Get the first child of the button.
    QDomNode n1 = n.firstChild();
    // Continue recursively if level is smaller than unfoldLevel-1.
    // This creates all buttons up to the maximum index unfoldLevel-1.
    if (unfoldLevel > level+1) {
        // Recursively create TOC GUI for all siblings of the current node.
        for (; !n1.isNull(); n1=n1.nextSibling())
            recursiveTocCreator(n1, level+1);
    }
    // Maximum unfold level is reached: Create drop down menus instead of buttons.
    else if (!n1.isNull()) {
        // Create a drop down menu for the next TOC level.
        QMenu* menu = new QMenu(e.tagName(), this);
        // Append the menu to the list of menus.
        menus.append(menu);
        // Get the page index for the main TOC entry.
        int const dest = pdf->destToSlide(e.attribute("DestinationName", ""));
        // Create a TocAction (menu entry) for the main TOC entry.
        TocAction* action = new TocAction(tocIndentStrings[0], e.tagName(), dest, this);
        // Hand over new page events from the action.
        connect(action, &TocAction::activated, this, &TocBox::sendNewPage);
        // Add the action to the menu.
        menu->addAction(action);
        // Add actions for the TOC subentries.
        for (; !n1.isNull(); n1 = n1.nextSibling()) {
            QDomElement const e = n1.toElement();
            if (e.isNull())
                continue;
            int const dest = pdf->destToSlide(e.attribute("DestinationName", ""));
            TocAction* action = new TocAction(tocIndentStrings[1], e.tagName(), dest, this);
            connect(action, &TocAction::activated, this, &TocBox::sendNewPage);
            menu->addAction(action);
        }
        // Add the menu to the button (which has the highest allowed TOC level).
        button->setMenu(menu);
    }
}

/// Destructor.
TocBox::~TocBox()
{
    // Disconnect all events.
    disconnect();
    // Delete all actions from all menus.
    for (QList<QMenu*>::const_iterator menu=menus.cbegin(); menu!=menus.cend(); menu++)
        qDeleteAll((*menu)->actions());
    // Delete all menus.
    qDeleteAll(menus);
    menus.clear();
    // Delete all buttons.
    qDeleteAll(buttons);
    buttons.clear();
    page_to_button.clear();
    // Delete layout and TOC document.
    delete layout;
    delete toc;
}

/// Focus on the TOC button for the section containing the given page.
void TocBox::focusCurrent(int const page)
{
    // Look up the current page in page_to_button.
    /// Iterator pointing to next TOC entry in page_to_button.
    QMap<int, int>::const_iterator const next_entry = page_to_button.upperBound(page);
    // If next_entry is the first entry. then focus on the first button.
    if (next_entry==page_to_button.cbegin())
        buttons.first()->setFocus();
    // Check whether next_entry-1 points to a valid button and focus on this button.
    else if (*(next_entry-1) < buttons.length())
        buttons[*(next_entry-1)]->setFocus();
    // Otherwise focus on the last button.
    else
        buttons.last()->setFocus();
}
