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

#include "overviewbox.h"

OverviewBox::OverviewBox(QWidget *parent) : QScrollArea(parent)
{
    client = new QWidget(this);
    setWidgetResizable(true);
    setWidget(client);
    layout = new QGridLayout(this);
    client->setLayout(layout);
    // TODO: handle keyboard shortcuts
}

OverviewBox::~OverviewBox()
{
    qDeleteAll(frames);
    frames.clear();
}

void OverviewBox::create(PdfDoc const* doc, int const columns)
{
    qDeleteAll(frames);
    frames.clear();
    // TODO: get the real width of the scroll area (instead of width()-16).
    client->setFixedWidth(width() - 16);
    this->columns = columns;
    QList<Poppler::Page*> const pages = *doc->getPages();
    double const frameWidth = double(client->width() - 2*columns - 2)/columns;
    int i=0;
    double resolution;
    for (QList<Poppler::Page*>::const_iterator page_it=pages.cbegin(); page_it!=pages.cend(); page_it++, i++) {
        OverviewFrame* frame = new OverviewFrame((*page_it)->index(), this);
        // TODO: handle beamer option notes on second screen
        frames.append(frame);
        resolution = 72*frameWidth / (*page_it)->pageSize().width();
        layout->addWidget(frame, i/columns, i%columns);
        frame->setPixmap(QPixmap::fromImage((*page_it)->renderToImage(resolution, resolution)));
        connect(frame, &OverviewFrame::activated, this, &OverviewBox::sendPageNumber);
    }
    outdated = false;
    show();
}
