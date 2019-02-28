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
#include <QDebug>
#include <QVBoxLayout>
#include <QDomDocument>
#include "tocbutton.h"
#include "tocaction.h"

class TocBox : public QWidget
{
    Q_OBJECT

private:
    QStringList const indentStrings = {"  ", "    ➤ ", "       - ", "          + "};
    QDomDocument const * toc = nullptr;
    int unfoldLevel = 2;
    QVBoxLayout* layout;
    QList<TocButton*> buttons;
    QList<QMenu*> menus;
    void recursiveTocCreator(QDomNode const& node, int const level);
    bool need_update = true;

public:
    TocBox(QWidget* parent = nullptr);
    ~TocBox();
    void createToc(QDomDocument const* toc);
    void setUnfoldLevel(int const level);
    bool needUpdate() {return need_update;}
    void setOutdated() {need_update=true;}
    bool hasToc() {return toc!=nullptr;}

signals:
    void sendDest(QString const & dest);
};

#endif // TOCBOX_H
