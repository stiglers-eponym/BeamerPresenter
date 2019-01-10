/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
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
