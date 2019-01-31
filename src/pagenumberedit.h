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

#ifndef PAGENUMBEREDIT_H
#define PAGENUMBEREDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>

class PageNumberEdit : public QLineEdit
{
    Q_OBJECT

public:
    PageNumberEdit(QWidget* parent = nullptr);
    ~PageNumberEdit();
    void setNumberOfPages(const int numberOfPages);

protected:
    void keyPressEvent(QKeyEvent* event);

private slots:
    void receiveReturnSignal();
    void receiveEditSignal(const QString string);

signals:
    void sendPageNumberReturn(int const pageNumber);
    void sendPageNumberEdit(int const pageNumber);
    void sendPageShiftEdit(int const shift);
    void sendNextSlideStart();
    void sendPreviousSlideEnd();
    void sendEscape();

private:
    int numberOfPages = 0;
};

#endif // PAGENUMBEREDIT_H
