#include "src/gui/tocbutton.h"

TOCbutton::TOCbutton(const QString &title, const int _page, QCheckBox *expand_button, QWidget *parent) :
    QPushButton(title, parent),
    expand_button(expand_button),
    page(_page)
{
    if (expand_button)
    {
        expand_button->setParent(this);
        expand_button->setChecked(true);
        // The expand_button likes to expand to the right,
        // effectively disabling the QPushButton lying under it.
        // Workaround: limit expand_button's size.
        expand_button->setMaximumWidth(24);
        connect(expand_button, &QCheckBox::pressed, this, &TOCbutton::expand);
        connect(expand_button, &QCheckBox::released, this, &TOCbutton::collapse);
        expand_button->setToolTip("uncheck to hide child layers");
    }
    connect(this, &QPushButton::clicked, this, [&]{emit sendNavigationEvent(page);});
    setToolTip("page " + QString::number(_page));
}

void TOCbutton::expand()
{
    show();
    if (expand_button)
    {
        expand_button->setToolTip("uncheck to hide child layers");
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
}

void TOCbutton::expand_full()
{
    show();
    if (expand_button)
    {
        expand_button->setToolTip("uncheck to hide child layers");
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
        expand_button->setToolTip("check to show child layers");
        expand_button->setChecked(false);
    }
    if (tree_child)
        tree_child->collapse_hide();
}

void TOCbutton::collapse_hide()
{
    hide();
    if (expand_button)
    {
        expand_button->hide();
        expand_button->setToolTip("check to show child layers");
        expand_button->setChecked(false);
    }
    if (tree_child)
        tree_child->collapse_hide();
    if (tree_next)
        tree_next->collapse_hide();
}
