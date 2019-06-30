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

// TODO: use BasicSlide for slide previews.

TocBox::TocBox(QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    this->setLayout(layout);
}

void TocBox::createToc(const QDomDocument *toc)
{
    if (!need_update || toc==nullptr || toc->isNull())
        return;
    if (this->toc != nullptr) {
        for (QList<QMenu*>::const_iterator menu=menus.cbegin(); menu!=menus.cend(); menu++)
            qDeleteAll((*menu)->actions());
        qDeleteAll(menus);
        menus.clear();
        qDeleteAll(buttons);
        buttons.clear();
        delete this->toc;
    }
    this->toc = toc;
    QDomNode n = toc->firstChild();
    while (!n.isNull()) {
        recursiveTocCreator(n, 0);
        n = n.nextSibling();
    }
    need_update = false;
}

void TocBox::setUnfoldLevel(const int level)
{
    if (unfoldLevel!=level) {
        need_update = true;
        unfoldLevel = level;
    }
}

void TocBox::recursiveTocCreator(QDomNode const& n, int const level)
{
    QDomElement e = n.toElement();
    if (e.isNull())
        return;
    QString const dest = e.attribute("DestinationName", "");
    TocButton * button = new TocButton(indentStrings[level] + e.tagName(), dest, this);
    connect(button, &TocButton::activated, this, [&](QString const dest){sendDest(dest);});
    buttons.append(button);
    layout->addWidget(button);
    QDomNode n1 = n.firstChild();
    if (unfoldLevel > level+1) {
        while (!n1.isNull()) {
            recursiveTocCreator(n1, level+1);
            n1 = n1.nextSibling();
        }
    }
    else if (!n1.isNull()) {
        if (QGuiApplication::platformName() == "wayland") {
            // Unstable patch because menus don't work in wayland (at least on my system)
            button->disconnect();
            QWidget * list = new QWidget(this);
            list->setLocale(button->locale());
            connect(button, &TocButton::clicked, list, &QWidget::show);
            QVBoxLayout * listLayout = new QVBoxLayout(list);
            list->setLayout(listLayout);
            list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            listLayout->setSizeConstraint(QVBoxLayout::SetMinimumSize);
            QString const dest = e.attribute("DestinationName", "");
            TocButton * button = new TocButton(indentStrings[0] + e.tagName(), dest, list);
            connect(button, &TocButton::activated, this, [&](QString const dest){sendDest(dest);});
            connect(button, &TocButton::activated, list, &QWidget::hide);
            listLayout->addWidget(button);
            while (!n1.isNull()) {
                QDomElement e = n1.toElement();
                if (e.isNull())
                    continue;
                QString const dest = e.attribute("DestinationName", "");
                TocButton * button = new TocButton(indentStrings[1] + e.tagName(), dest, list);
                connect(button, &TocButton::activated, this, [&](QString const dest){sendDest(dest);});
                connect(button, &TocButton::activated, list, &QWidget::hide);
                listLayout->addWidget(button);
                n1 = n1.nextSibling();
            }
            list->setAutoFillBackground(true);
            list->hide();
        }
        else {
            QMenu* menu = new QMenu(dest, this);
            menus.append(menu);
            QString const dest = e.attribute("DestinationName", "");
            TocAction * action = new TocAction(indentStrings[0] + e.tagName(), dest, this);
            connect(action, &TocAction::activated, this, [&](QString const dest){sendDest(dest);});
            menu->addAction(action);
            while (!n1.isNull()) {
                QDomElement e = n1.toElement();
                if (e.isNull())
                    continue;
                QString const dest = e.attribute("DestinationName", "");
                TocAction * action = new TocAction(indentStrings[1] + e.tagName(), dest, this);
                connect(action, &TocAction::activated, this, [&](QString const dest){sendDest(dest);});
                menu->addAction(action);
                n1 = n1.nextSibling();
            }
            button->setMenu(menu);
        }
    }
}

TocBox::~TocBox()
{
    disconnect();
    for (QList<QMenu*>::const_iterator menu=menus.cbegin(); menu!=menus.cend(); menu++)
        qDeleteAll((*menu)->actions());
    qDeleteAll(menus);
    qDeleteAll(buttons);
    buttons.clear();
    delete layout;
    delete toc;
}
