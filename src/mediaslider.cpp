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

#include "mediaslider.h"

MediaSlider::MediaSlider(QWidget * parent) : QSlider(Qt::Horizontal, parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setRange(0, 10);
    setSingleStep(1);
    connect(this, &QSlider::valueChanged, this, &MediaSlider::emitSliderMoved);
}

MediaSlider::~MediaSlider()
{
    disconnect();
}


void MediaSlider::setMaximum(qint64 const max)
{
    QSlider::setMaximum(int(max/100));
}

void MediaSlider::setValue(const qint64 value)
{
    QSlider::setValue(int(value/100));
}

void MediaSlider::emitSliderMoved(int const value)
{
    emit sliderMoved(100*qint64(value));
}

void MediaSlider::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            emit sendEscapeEvent();
            break;
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Up:
        case Qt::Key_Down:
            emit sendKeyEvent(event);
            break;
        default:
            QSlider::keyPressEvent(event);
            break;
    }
    event->accept();
}
