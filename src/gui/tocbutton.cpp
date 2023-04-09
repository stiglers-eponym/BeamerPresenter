// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QPushButton>
#include <QString>
#include "src/gui/tocbutton.h"

TOCbutton::TOCbutton(const QString &title, const int _page, QPushButton *expand_button, QWidget *parent) :
    QPushButton(title, parent),
    expand_button(expand_button),
    page(_page)
{
    setMinimumSize(40, 26);
    setMaximumHeight(48);
    setFlat(true);
    setStyleSheet("text-align:left;padding-left:8px;");
    if (expand_button)
    {
        expand_button->setParent(this);
        expand_button->setChecked(true);
        // The expand_button likes to expand to the right,
        // effectively disabling the QPushButton lying under it.
        // Workaround: limit expand_button's size.
        expand_button->setMaximumSize(32, 48);
        connect(expand_button, &QPushButton::toggled, this, &TOCbutton::toggleVisibility);
        expand_button->setToolTip(tr("uncheck to hide child layers"));
    }
    connect(this, &QPushButton::clicked, this, [&]{emit sendNavigationEvent(page);});
    setToolTip(tr("page ") + QString::number(_page));
}

TOCbutton::~TOCbutton()
{
    delete expand_button;
    delete tree_child;
    delete tree_next;
}

void TOCbutton::expand()
{
    show();
    if (expand_button)
    {
        expand_button->setToolTip(tr("uncheck to hide child layers"));
        expand_button->show();
        expand_button->setChecked(true);
    }
    TOCbutton *child = tree_child;
    while (child)
    {
        child->show();
        if (child->expand_button)
            child->expand_button->show();
        child = child->tree_next;
    }
    parentWidget()->updateGeometry();
}

void TOCbutton::expand_full()
{
    show();
    if (expand_button)
    {
        expand_button->setToolTip(tr("uncheck to hide child layers"));
        expand_button->show();
        expand_button->setChecked(true);
    }
    TOCbutton *child = tree_child;
    while (child)
    {
        child->expand_full();
        child = child->tree_next;
    }
}

void TOCbutton::collapse()
{
    if (expand_button)
    {
        expand_button->setToolTip(tr("check to show child layers"));
        expand_button->setChecked(false);
    }
    if (tree_child)
        tree_child->collapse_hide();
    parentWidget()->updateGeometry();
}

void TOCbutton::collapse_hide()
{
    hide();
    if (expand_button)
    {
        expand_button->hide();
        expand_button->setToolTip(tr("check to show child layers"));
        expand_button->setChecked(false);
    }
    if (tree_child)
        tree_child->collapse_hide();
    if (tree_next)
        tree_next->collapse_hide();
}
