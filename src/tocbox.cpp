#include "tocbox.h"

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
        for (QList<QMenu*>::iterator menu=menus.begin(); menu!=menus.end(); menu++)
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
        QMenu* menu = new QMenu(dest, this);
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

TocBox::~TocBox()
{
    disconnect();
    for (QList<QMenu*>::iterator menu=menus.begin(); menu!=menus.end(); menu++)
        qDeleteAll((*menu)->actions());
    qDeleteAll(menus);
    qDeleteAll(buttons);
    buttons.clear();
    delete layout;
    delete toc;
}
