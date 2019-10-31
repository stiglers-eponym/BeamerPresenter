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
#include "pdfdoc.h"
#include "enumerates.h"

class OverviewBox : public QScrollArea
{
    Q_OBJECT

private:
    QGridLayout* layout;
    QList<OverviewFrame*> frames;
    QWidget* client;
    bool outdated=true;
    quint8 columns;
    int focused = 0;

public:
    explicit OverviewBox(QWidget* parent = nullptr);
    ~OverviewBox();
    void create(PdfDoc const* doc, quint8 const columns = 5, PagePart const pagePart = PagePart::FullPage);
    bool needsUpdate() const {return outdated;}
    void setOutdated() {outdated=true;}
    void setFocused(int const page);

signals:
    void sendPageNumber(int const page);
    void sendReturn();

public slots:
};

#endif // OVERVIEWBOX_H
