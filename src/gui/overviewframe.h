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

#ifndef OVERVIEWFRAME_H
#define OVERVIEWFRAME_H

#include <QtDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <poppler-qt5.h>

class OverviewFrame : public QLabel
{
    Q_OBJECT

private:
    int page;

protected:
    void mousePressEvent(QMouseEvent* event) override;

public:
    OverviewFrame(int const page, QWidget* parent = nullptr);
    void activate();
    void deactivate();

signals:
    void activated(int const page);
    void sendReturn();
};

#endif // OVERVIEWFRAME_H
