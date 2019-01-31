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

#include "pagenumberedit.h"

PageNumberEdit::PageNumberEdit(QWidget* parent) : QLineEdit(parent)
{
    connect(this, &PageNumberEdit::textChanged, this, &PageNumberEdit::receiveEditSignal);
}

PageNumberEdit::~PageNumberEdit()
{
    disconnect();
}

void PageNumberEdit::setNumberOfPages(const int numberOfPages)
{
    this->numberOfPages = numberOfPages;
}

void PageNumberEdit::receiveReturnSignal()
{
    emit sendPageNumberReturn( text().toInt() - 1 );
}

void PageNumberEdit::receiveEditSignal(const QString string)
{
    int pageNumber = string.toInt();
    if ((pageNumber > 0) && (pageNumber <= numberOfPages))
        emit sendPageNumberEdit( pageNumber - 1 );
}

void PageNumberEdit::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_PageDown:
            {
                int page = text().toInt();
                if (page < numberOfPages) {
                    emit sendPageNumberReturn(page);
                    emit sendPageNumberEdit(page);
                }
            }
            break;
        case Qt::Key_PageUp:
            {
                int page = text().toInt() - 2;
                if (page >= 0) {
                    emit sendPageNumberReturn(page);
                    emit sendPageNumberEdit(page);
                }
            }
            break;
        case Qt::Key_Right:
            emit sendPageShiftEdit(1);
            break;
        case Qt::Key_Down:
            emit sendNextSlideStart();
            break;
        case Qt::Key_Left:
            emit sendPageShiftEdit(-1);
            break;
        case Qt::Key_Up:
            emit sendPreviousSlideEnd();
            break;
        case Qt::Key_End:
            emit sendPageNumberEdit(numberOfPages - 1);
            break;
        case Qt::Key_Home:
            emit sendPageNumberEdit(0);
            break;
        case Qt::Key_Return:
            emit sendPageNumberReturn(text().toInt() - 1);
            emit sendEscape();
            break;
        case Qt::Key_Escape:
            emit sendEscape();
            break;
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Control:
            QLineEdit::keyPressEvent(event);
            break;
    }
    event->accept();
}
