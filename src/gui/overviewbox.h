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

#ifndef OVERVIEWBOX_H
#define OVERVIEWBOX_H

#include <QtDebug>
#include <QScrollArea>
#include <QGridLayout>
#include "overviewframe.h"
#include "../pdf/pdfdoc.h"
#include "../enumerates.h"

class OverviewBox : public QScrollArea
{
    Q_OBJECT

private:
    QGridLayout* layout;
    QList<OverviewFrame*> frames;
    QWidget* client;
    bool outdated = true;
    quint8 columns = 5;
    int focused = 0;

protected:
    void keyPressEvent(QKeyEvent* event) override {event->setAccepted(false);}

public:
    explicit OverviewBox(QWidget* parent = nullptr);
    ~OverviewBox();
    void create(PdfDoc const* doc, PagePart const pagePart = PagePart::FullPage);
    void setColumns(quint8 const cols) {columns = cols;}
    bool needsUpdate() const {return outdated;}
    void setOutdated() {outdated=true;}
    void setFocused(int const page);
    void moveFocusDown() {setFocused(focused+columns);}
    void moveFocusUp() {setFocused(focused-columns);}
    void moveFocusLeft() {setFocused(focused-1);}
    void moveFocusRight() {setFocused(focused+1);}
    int getPage() const {return focused;}

signals:
    void sendPageNumber(int const page);
    void sendReturn();

public slots:
};

#endif // OVERVIEWBOX_H
