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

#include "presentationwidget.h"

PresentationWidget::PresentationWidget(QWidget* parent) : PageWidget(parent)
{
    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PageWidget::timeoutSignal);
}

PresentationWidget::~PresentationWidget()
{
    timer.stop();
    timeoutTimer->stop();
    delete timeoutTimer;
}

void PresentationWidget::paintEvent(QPaintEvent*)
{
    if (elapsed >= transition_duration) {
        timer.stop();
        painter.begin(this);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        picinit = pixmap;
        return;
    }
    elapsed += dt;
    painter.begin(this);
    (this->*paint)();
    painter.end();
}

void PresentationWidget::endAnimation()
{
    timeoutTimer->stop();
    if (timer.isActive()) {
        elapsed = transition_duration;
        picinit = pixmap;
    }
}

void PresentationWidget::setDuration()
{
    duration = page->duration(); // duration of the current page in s
    // For durations longer than the minimum animation delay: use the duration
    if (duration*1000 > minimumAnimationDelay) {
        timeoutTimer->start(int(1000*duration));
        if (duration < 0.5)
            update();
    }
    // For durations of approximately 0: use the minimum animation delay
    else if (duration > -1e-6) {
        timeoutTimer->start(minimumAnimationDelay);
        update();
    }
}

void PresentationWidget::animate() {
    // TODO: Test and implement more transitions
    Poppler::PageTransition const* transition = page->transition();
    timer.stop();
    if (pixmap.isNull()) {
        transition_duration = 0;
        return;
    }
    picwidth = pixmap.width();
    picheight = pixmap.height();
    if (transition == nullptr || picinit.isNull() || (duration>-1e-6 && 250*duration < dt)) {
        transition_duration = 0;
        painter.begin(this);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        picinit = pixmap;
        return;
    }
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        transition_duration = 0;
        painter.begin(this);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        picinit = pixmap;
        return;
    case Poppler::PageTransition::Split:
        qDebug () << "Transition split";
        if (transition->alignment() == Poppler::PageTransition::Horizontal) {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &PresentationWidget::paintSplitHI;
            else
                paint = &PresentationWidget::paintSplitHO;
        }
        else {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &PresentationWidget::paintSplitVI;
            else
                paint = &PresentationWidget::paintSplitVO;
        }
        break;
    case Poppler::PageTransition::Blinds:
        qDebug () << "Transition blinds";
        if (transition->alignment() == Poppler::PageTransition::Horizontal)
            paint = &PresentationWidget::paintBlindsH;
        else
            paint = &PresentationWidget::paintBlindsV;
        break;
    case Poppler::PageTransition::Box:
        qDebug () << "Transition box";
        if (transition->direction() == Poppler::PageTransition::Inward)
            paint = &PresentationWidget::paintBoxI;
        else
            paint = &PresentationWidget::paintBoxO;
        break;
    case Poppler::PageTransition::Wipe:
        {
        qDebug () << "Transition wipe" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationWidget::paintWipeRight;
        else if (angle < 135)
            paint = &PresentationWidget::paintWipeUp;
        else if (angle < 225)
            paint = &PresentationWidget::paintWipeLeft;
        else
            paint = &PresentationWidget::paintWipeDown;
        }
        break;
    case Poppler::PageTransition::Dissolve:
        qDebug () << "Transition dissolve";
        paint = &PresentationWidget::paintDissolve;
        break;
    case Poppler::PageTransition::Glitter:
        qWarning () << "Unsupported transition type: Glitter";
        paint = &PresentationWidget::paintGlitter;
        break;
    case Poppler::PageTransition::Fly:
        qDebug () << "Transition fly";
        paint = &PresentationWidget::paintFly;
        break;
    case Poppler::PageTransition::Push:
        {
        qDebug () << "Transition push" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationWidget::paintPushRight;
        else if (angle < 135)
            paint = &PresentationWidget::paintPushUp;
        else if (angle < 225)
            paint = &PresentationWidget::paintPushLeft;
        else
            paint = &PresentationWidget::paintPushDown;
        }
        break;
    case Poppler::PageTransition::Cover:
        qWarning () << "Unsupported transition of type cover";
        paint = &PresentationWidget::paintCover;
        break;
    case Poppler::PageTransition::Uncover:
        qWarning () << "Unsupported transition of type uncover";
        paint = &PresentationWidget::paintUncover;
        break;
    case Poppler::PageTransition::Fade:
        qDebug () << "Transition fade";
        paint = &PresentationWidget::paintFade;
        break;
    }
    transition_duration = static_cast<int>(1000*transition->durationReal());
    elapsed = 0;
    painter.begin(this);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.end();
    timer.start(dt);
}

void PresentationWidget::paintWipeUp()
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, 0, split);
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, -1, -1);
}

void PresentationWidget::paintWipeDown()
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, 0, split);
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, -1, -1);
}

void PresentationWidget::paintWipeLeft()
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, split, -1);
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, -1, -1);
}

void PresentationWidget::paintWipeRight()
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, split, -1);
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, -1, -1);
}

void PresentationWidget::paintBlindsV()
{
    int width = (picwidth*elapsed)/(n_blinds*transition_duration);
    if (width==0)
        width = 1;
    painter.drawPixmap(shiftx, shifty, picinit);
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx+i*picwidth/n_blinds, shifty, pixmap, i*picwidth/n_blinds, 0, width, -1);
}

void PresentationWidget::paintBlindsH()
{
    //int const shift = elapsed>dt ? ((elapsed-dt)*picheight)/(n_blinds*transition_duration) - 1 : 0;
    int height = (picheight*elapsed)/(n_blinds*transition_duration);
    if (height==0)
        height = 1;
    painter.drawPixmap(shiftx, shifty, picinit);
    for (int i=0; i<n_blinds; i++) {
        painter.drawPixmap(shiftx, i*picheight/n_blinds+shifty, pixmap, 0, i*picheight/n_blinds, -1, height);
    }
}

void PresentationWidget::paintBoxO()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight-height)/2, pixmap, (picwidth-width)/2, (picheight-height)/2, width, height);
}

void PresentationWidget::paintBoxI()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    if (height*width==0)
        painter.drawPixmap(shiftx, shifty, pixmap);
    else {
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.drawPixmap(shiftx+width/2, shifty+height/2, picinit, width/2, height/2, picwidth-width, picheight-height);
    }
}

void PresentationWidget::paintSplitHO()
{
    int const height = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight+height)/2, picinit, 0, (picheight+height)/2, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight-height)/2, pixmap, 0, (picheight-height)/2, -1, height);
}

void PresentationWidget::paintSplitVO()
{
    int const width = (elapsed*picwidth)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth+width)/2, shifty, picinit, (picwidth+width)/2, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, pixmap, (picwidth-width)/2, 0, width, -1);
}

void PresentationWidget::paintSplitHI()
{
    int const height = picheight - (elapsed*picheight)/transition_duration;
    if (height==0)
        painter.drawPixmap(shiftx, shifty, pixmap);
    else {
        painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, -1, (picheight-height)/2+1);
        painter.drawPixmap(shiftx, shifty+(picheight+height)/2, pixmap, 0, (picheight+height)/2, -1, (picheight-height)/2+1);
        painter.drawPixmap(shiftx, shifty+(picheight-height)/2, picinit, 0, (picheight-height)/2, -1, height);
    }
}

void PresentationWidget::paintSplitVI()
{
    int const width = picwidth - (elapsed*picwidth)/transition_duration;
    if (width==0)
        painter.drawPixmap(shiftx, shifty, pixmap);
    else {
        painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, (picwidth-width)/2+1, -1);
        painter.drawPixmap(shiftx+(picwidth+width)/2, shifty, pixmap, (picwidth+width)/2, 0, (picwidth-width)/2+1, -1);
        painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, picinit, (picwidth-width)/2, 0, width, -1);
    }
}

void PresentationWidget::paintDissolve()
{
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
}

void PresentationWidget::paintGlitter()
{
    // TODO
    painter.drawPixmap(shiftx, shifty, pixmap);
    transition_duration = 0;
}

void PresentationWidget::paintFly()
{
    // TODO
    painter.drawPixmap(shiftx, shifty, pixmap);
    transition_duration = 0;
}

void PresentationWidget::paintPushUp()
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, picheight- split, 0, 0);
    painter.drawPixmap(shiftx, split, pixmap, 0, 0, 0, split);
}

void PresentationWidget::paintPushDown()
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split, picinit, 0, 0, 0, picheight- split);
    painter.drawPixmap(shiftx, shifty, pixmap, 0, picheight - split, 0, 0);
}

void PresentationWidget::paintPushLeft()
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, picwidth- split, 0, 0, 0);
    painter.drawPixmap(shiftx+split, shifty, pixmap, 0, 0, split, 0);
}

void PresentationWidget::paintPushRight()
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, picinit, 0, 0, picwidth- split, 0);
    painter.drawPixmap(shiftx, shifty, pixmap, picwidth - split, 0, 0, 0);
}

void PresentationWidget::paintCover()
{
    // TODO
    painter.drawPixmap(shiftx, shifty, pixmap);
    transition_duration = 0;
}

void PresentationWidget::paintUncover()
{
    // TODO
    painter.drawPixmap(shiftx, shifty, pixmap);
    transition_duration = 0;
}


void PresentationWidget::paintFade()
{
    painter.setOpacity(static_cast<double>(transition_duration - elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
}
