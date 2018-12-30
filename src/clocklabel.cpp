/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "clocklabel.h"

ClockLabel::ClockLabel(QWidget* parent) : QLabel(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClockLabel::showTime);
    timer->start(1000);
    showTime();
}

void ClockLabel::showTime()
{
    setText(QTime::currentTime().toString("hh:mm:ss"));
}
