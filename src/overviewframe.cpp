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

#include "overviewframe.h"

OverviewFrame::OverviewFrame(int const page, quint8 const columns, QWidget* parent) : QLabel(parent)
{
    this->page = page;
    this->columns = columns;
    setAlignment(Qt::AlignCenter);
}

void OverviewFrame::mousePressEvent(QMouseEvent* event)
{
    emit activated(page);
    emit selected(page);
    event->accept();
}

void OverviewFrame::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Return:
        emit selected(page);
        emit sendReturn();
        break;
    case Qt::Key_Left:
        emit activated(page-1);
        break;
    case Qt::Key_Right:
        emit activated(page+1);
        break;
    case Qt::Key_Up:
        emit activated(page-columns);
        break;
    case Qt::Key_Down:
        emit activated(page+columns);
        break;
    case Qt::Key_Home:
        emit activated(0);
        break;
    case Qt::Key_End:
        // Go to page "infinity"
        emit activated(1073741824);
        break;
    case Qt::Key_PageUp:
        emit activated(page-1);
        emit selected(page-1);
        break;
    case Qt::Key_PageDown:
        emit activated(page+1);
        emit selected(page+1);
        break;
    default:
        event->setAccepted(false);
    }
}

void OverviewFrame::activate()
{
    setStyleSheet("QLabel {border: 4px solid red;}");
    setFocus();
}

void OverviewFrame::deactivate()
{
    setStyleSheet("QLabel {border: 0px;}");
}
