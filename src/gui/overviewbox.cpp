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

OverviewBox::OverviewBox(QWidget *parent) :
    QScrollArea(parent),
    layout(new QGridLayout(this)),
    client(new QWidget(this))
{
    setWidgetResizable(true);
    setWidget(client);
    client->setLayout(layout);
    //setShortcutEnabled(false); // TODO: check what this does
    // TODO: handle keyboard shortcuts
    QScroller::grabGesture(this);
}

OverviewBox::~OverviewBox()
{
    qDeleteAll(frames);
    frames.clear();
}

void OverviewBox::create(PdfDoc const* doc, PagePart const pagePart)
{
    qDeleteAll(frames);
    frames.clear();
    // TODO: get the real width of the scroll area (instead of width()-16).
    client->setFixedWidth(width() - 16);
    QList<Poppler::Page*> const pages = *doc->getPages();
    double const frameWidth = double(client->width() - 2*columns - 2)/columns;
    int i=0;
    double resolution;
    for (QList<Poppler::Page*>::const_iterator page_it=pages.cbegin(); page_it!=pages.cend(); page_it++, i++) {
        OverviewFrame* frame = new OverviewFrame(i, this);
        frames.append(frame);
        resolution = 72*frameWidth / (*page_it)->pageSize().width();
        layout->addWidget(frame, i/columns, i%columns);
        QPixmap pixmap;
        if (pagePart == FullPage)
            pixmap = QPixmap::fromImage((*page_it)->renderToImage(resolution, resolution));
        else {
            QImage image = (*page_it)->renderToImage(2*resolution, 2*resolution);
            if (pagePart == LeftHalf)
                pixmap = QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
            else
                pixmap = QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
        }
        frame->setPixmap(pixmap);
        connect(frame, &OverviewFrame::activated, this, &OverviewBox::sendPageNumber);
        connect(frame, &OverviewFrame::activated, this, &OverviewBox::setFocused);
    }
    outdated = false;
    show();
}

void OverviewBox::setFocused(int page)
{
    if (page < 0)
        page = 0;
    else if (page >= frames.length())
        page = frames.length()-1;
    frames[focused]->deactivate();
    focused = page;
    frames[focused]->activate();
    ensureWidgetVisible(frames[focused]);
}
