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

#ifndef MEDIASLIDER_H
#define MEDIASLIDER_H

#include <QObject>
#include <QWidget>
#include <QSlider>
#include <QKeyEvent>
#include <iostream>

class MediaSlider : public QSlider
{
    Q_OBJECT

public:
    MediaSlider(QWidget* parent = nullptr);
    ~MediaSlider();
    void setMaximum(qint64 const max);
    void setValue(qint64 const value);

protected:
    void keyPressEvent(QKeyEvent* event);

public slots:
    void emitSliderMoved(int const value);

signals:
    void sliderMoved(qint64 const value);
    void sendKeyEvent(QKeyEvent* event);
    void sendEscapeEvent();
};

#endif // MEDIASLIDER_H
