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
    connect(this, &QSlider::sliderMoved, this, &MediaSlider::emitSliderMoved);
}

MediaSlider::~MediaSlider()
{
    disconnect(this, &QSlider::sliderMoved, this, &MediaSlider::emitSliderMoved);
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
