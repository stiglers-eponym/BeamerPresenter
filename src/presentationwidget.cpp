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
    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PageWidget::timeoutSignal);
    // TODO: Check whether disabling partial updates and redrawing everything would be more efficient.
    setUpdateBehavior(PartialUpdate);
}

PresentationWidget::~PresentationWidget()
{
    timer.stop();
    timeoutTimer->stop();
    delete timeoutTimer;
}

void PresentationWidget::paintGL()
{
    if (elapsed >= transition_duration) {
        timer.stop();
        painter.begin(this);
        painter.fillRect(rect(), background);
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
    if (pixmap.isNull())
        return;
    picwidth = pixmap.width();
    picheight = pixmap.height();
    if (transition == nullptr || picinit.isNull() || (duration>-1e-6 && 250*duration < dt)) {
        makeCurrent();
        painter.begin(this);
        painter.fillRect(rect(), background);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        doneCurrent();
        picinit = pixmap;
        return;
    }
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        makeCurrent();
        painter.begin(this);
        painter.fillRect(rect(), background);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        doneCurrent();
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
    makeCurrent();
    painter.begin(this);
    painter.fillRect(rect(), background);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.end();
    doneCurrent();
    timer.start(dt);
}

void PresentationWidget::paintWipeUp()
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, 0, split + dt*picheight/transition_duration + 1);
}

void PresentationWidget::paintWipeDown()
{
    int const split = elapsed*picheight/transition_duration;
    int const start = split - dt*picheight/transition_duration - 1;
    painter.drawPixmap(shiftx, start+shifty, pixmap, 0, start, 0, split);
}

void PresentationWidget::paintWipeLeft()
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, split + dt*picwidth/transition_duration + 1, 0);
}

void PresentationWidget::paintWipeRight()
{
    int const split = elapsed*picwidth/transition_duration;
    int const start = split - dt*picwidth/transition_duration - 1;
    painter.drawPixmap(start+shiftx, shifty, pixmap, start, 0, split, 0);
}

void PresentationWidget::paintBlindsV()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picwidth)/(n_blinds*transition_duration) - 1 : 0;
    int width = (picwidth*elapsed)/(n_blinds*transition_duration) - shift;
    if (width==0)
        width = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx+i*picwidth/n_blinds+shift, shifty, pixmap, i*picwidth/n_blinds+shift, 0, width, -1);
}

void PresentationWidget::paintBlindsH()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picheight)/(n_blinds*transition_duration) - 1 : 0;
    int height = (picheight*elapsed)/(n_blinds*transition_duration)-shift;
    if (height==0)
        height = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx, i*picheight/n_blinds+shift+shifty, pixmap, 0, i*picheight/n_blinds+shift, -1, height);
}

void PresentationWidget::paintBoxO()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight-height)/2, pixmap, (picwidth-width)/2, (picheight-height)/2, width, height);

    // The following code should be more efficient, but it caused rendering errors when I tried it.
    //int const width = (elapsed*picwidth)/transition_duration;
    //int const height = (elapsed*picheight)/transition_duration;
    //int const dw = (dt*picwidth)/(2*transition_duration);
    //int const dh = (dt*picheight)/(2*transition_duration);
    //painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight-height)/2, pixmap, (picwidth-width)/2, (picheight-height)/2, width, dh);
    //painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight-height)/2+dh, pixmap, (picwidth-width)/2, (picheight-height)/2, dw, height-2*dh);
    //painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight+height)/2-dh, pixmap, (picwidth-width)/2, (picheight+height)/2-dh, width, dh);
    //painter.drawPixmap(shiftx+(picwidth+width)/2-dw, shifty+(picheight-height)/2, pixmap, (picwidth+width)/2-dw, (picheight-height)/2, dw, height-2*dh);
}

void PresentationWidget::paintBoxI()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    int const dw = (dt*picwidth)/(2*transition_duration)+1;
    int const dh = (dt*picheight)/(2*transition_duration)+1;
    painter.drawPixmap(shiftx+width/2-dw, shifty+height/2-dh, pixmap, width/2-dw, height/2-dh, picwidth-width+2*dw, dh);
    painter.drawPixmap(shiftx+width/2-dw, shifty+height/2, pixmap, width/2-dw, height/2, dw, picheight-height+1);
    painter.drawPixmap(shiftx+width/2-dw, shifty+picheight-height/2, pixmap, width/2-dw, picheight-height/2, picwidth-width+2*dw, dh);
    painter.drawPixmap(shiftx+picwidth-width/2, shifty+height/2, pixmap, picwidth-width/2, height/2, dw, picheight-height+1);
}

void PresentationWidget::paintSplitHO()
{
    int const height = (elapsed*picheight)/transition_duration;
    int const dh = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx, shifty+(picheight-height)/2, pixmap, 0, (picheight-height)/2, -1, dh);
    painter.drawPixmap(shiftx, shifty+(picheight+height)/2-dh, pixmap, 0, (picheight+height)/2-dh, -1, dh);
}

void PresentationWidget::paintSplitVO()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const dw = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, pixmap, (picwidth-width)/2, 0, dw, -1);
    painter.drawPixmap(shiftx+(picwidth+width)/2-dw, shifty, pixmap, (picwidth+width)/2-dw, 0, dw, -1);
}

void PresentationWidget::paintSplitHI()
{
    int const height = (elapsed*picheight)/transition_duration;
    int const dh = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx, shifty+height/2-dh, pixmap, 0, height/2-dh, -1, dh);
    painter.drawPixmap(shiftx, shifty+picheight-height/2, pixmap, 0, picheight-height/2, -1, dh);
}

void PresentationWidget::paintSplitVI()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const dw = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx+width/2-dw, shifty, pixmap, width/2-dw, 0, dw, -1);
    painter.drawPixmap(shiftx+picwidth-width/2, shifty, pixmap, picwidth-width/2, 0, dw, -1);
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
    elapsed = transition_duration;
}

void PresentationWidget::paintFly()
{
    // TODO
    elapsed = transition_duration;
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
    elapsed = transition_duration;
}

void PresentationWidget::paintUncover()
{
    // TODO
    elapsed = transition_duration;
}


void PresentationWidget::paintFade()
{
    painter.drawRect(shiftx, shifty, picwidth, picheight);
    painter.setOpacity(static_cast<double>(transition_duration - elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
}
