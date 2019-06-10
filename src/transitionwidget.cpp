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

#include "transitionwidget.h"

TransitionWidget::TransitionWidget(QWidget* parent) : PageLabel(parent)
{
    isPresentation = true;
    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
}

void TransitionWidget::paintEvent(QPaintEvent *event)
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

void TransitionWidget::animate() {
    // TODO: Test and implement more transitions
    Poppler::PageTransition const* transition = page->transition();
    timer.stop();
    if (pixmap.isNull())
        return;
    picwidth = pixmap.width();
    picheight = pixmap.height();
    if (transition == nullptr || picinit.isNull()) {
        painter.begin(this);
        painter.fillRect(rect(), background);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        picinit = pixmap;
        return;
    }
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        painter.begin(this);
        painter.fillRect(rect(), background);
        painter.drawPixmap(shiftx, shifty, pixmap);
        painter.end();
        picinit = pixmap;
        return;
    case Poppler::PageTransition::Split:
        qDebug () << "Transition split";
        if (transition->alignment() == Poppler::PageTransition::Horizontal) {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &TransitionWidget::paintSplitHI;
            else
                paint = &TransitionWidget::paintSplitHO;
        }
        else {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &TransitionWidget::paintSplitVI;
            else
                paint = &TransitionWidget::paintSplitVO;
        }
        break;
    case Poppler::PageTransition::Blinds:
        qDebug () << "Transition blinds";
        if (transition->alignment() == Poppler::PageTransition::Horizontal)
            paint = &TransitionWidget::paintBlindsH;
        else
            paint = &TransitionWidget::paintBlindsV;
        break;
    case Poppler::PageTransition::Box:
        qDebug () << "Transition box";
        if (transition->direction() == Poppler::PageTransition::Inward)
            paint = &TransitionWidget::paintBoxI;
        else
            paint = &TransitionWidget::paintBoxO;
        break;
    case Poppler::PageTransition::Wipe:
        {
        qDebug () << "Transition wipe" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &TransitionWidget::paintWipeRight;
        else if (angle < 135)
            paint = &TransitionWidget::paintWipeUp;
        else if (angle < 225)
            paint = &TransitionWidget::paintWipeLeft;
        else
            paint = &TransitionWidget::paintWipeDown;
        }
        break;
    case Poppler::PageTransition::Dissolve:
        qDebug () << "Transition dissolve";
        paint = &TransitionWidget::paintDissolve;
        break;
    case Poppler::PageTransition::Glitter:
        qWarning () << "Unsupported transition type: Glitter";
        paint = &TransitionWidget::paintGlitter;
        break;
    case Poppler::PageTransition::Fly:
        qDebug () << "Transition fly";
        paint = &TransitionWidget::paintFly;
        break;
    case Poppler::PageTransition::Push:
        {
        qDebug () << "Transition push" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &TransitionWidget::paintPushRight;
        else if (angle < 135)
            paint = &TransitionWidget::paintPushUp;
        else if (angle < 225)
            paint = &TransitionWidget::paintPushLeft;
        else
            paint = &TransitionWidget::paintPushDown;
        }
        break;
    case Poppler::PageTransition::Cover:
        qWarning () << "Unsupported transition of type cover";
        paint = &TransitionWidget::paintCover;
        break;
    case Poppler::PageTransition::Uncover:
        qWarning () << "Unsupported transition of type uncover";
        paint = &TransitionWidget::paintUncover;
        break;
    case Poppler::PageTransition::Fade:
        qDebug () << "Transition fade";
        paint = &TransitionWidget::paintFade;
        break;
    }
    transition_duration = static_cast<int>(1000*transition->durationReal());
    elapsed = 0;
    painter.begin(this);
    painter.fillRect(rect(), background);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.end();
    timer.start(dt);
}

void TransitionWidget::paintWipeUp()
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, 0, split + dt*picheight/transition_duration + 1);
}

void TransitionWidget::paintWipeDown()
{
    int const split = elapsed*picheight/transition_duration;
    int const start = split - dt*picheight/transition_duration - 1;
    painter.drawPixmap(shiftx, start+shifty, pixmap, 0, start, 0, split);
}

void TransitionWidget::paintWipeLeft()
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, split + dt*picwidth/transition_duration + 1, 0);
}

void TransitionWidget::paintWipeRight()
{
    int const split = elapsed*picwidth/transition_duration;
    int const start = split - dt*picwidth/transition_duration - 1;
    painter.drawPixmap(start+shiftx, shifty, pixmap, start, 0, split, 0);
}

void TransitionWidget::paintBlindsV()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picwidth)/(n_blinds*transition_duration) - 1 : 0;
    int width = (picwidth*elapsed)/(n_blinds*transition_duration) - shift;
    if (width==0)
        width = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx+i*picwidth/n_blinds+shift, shifty, pixmap, i*picwidth/n_blinds+shift, 0, width, -1);
}

void TransitionWidget::paintBlindsH()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picheight)/(n_blinds*transition_duration) - 1 : 0;
    int height = (picheight*elapsed)/(n_blinds*transition_duration)-shift;
    if (height==0)
        height = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx, i*picheight/n_blinds+shift+shifty, pixmap, 0, i*picheight/n_blinds+shift, -1, height);
}

void TransitionWidget::paintBoxO()
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

void TransitionWidget::paintBoxI()
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

void TransitionWidget::paintSplitHO()
{
    int const height = (elapsed*picheight)/transition_duration;
    int const dh = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx, shifty+(picheight-height)/2, pixmap, 0, (picheight-height)/2, -1, dh);
    painter.drawPixmap(shiftx, shifty+(picheight+height)/2-dh, pixmap, 0, (picheight+height)/2-dh, -1, dh);
}

void TransitionWidget::paintSplitVO()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const dw = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, pixmap, (picwidth-width)/2, 0, dw, -1);
    painter.drawPixmap(shiftx+(picwidth+width)/2-dw, shifty, pixmap, (picwidth+width)/2-dw, 0, dw, -1);
}

void TransitionWidget::paintSplitHI()
{
    int const height = (elapsed*picheight)/transition_duration;
    int const dh = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx, shifty+height/2-dh, pixmap, 0, height/2-dh, -1, dh);
    painter.drawPixmap(shiftx, shifty+picheight-height/2, pixmap, 0, picheight-height/2, -1, dh);
}

void TransitionWidget::paintSplitVI()
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const dw = (dt*picheight)/transition_duration + 1;
    painter.drawPixmap(shiftx+width/2-dw, shifty, pixmap, width/2-dw, 0, dw, -1);
    painter.drawPixmap(shiftx+picwidth-width/2, shifty, pixmap, picwidth-width/2, 0, dw, -1);
}

void TransitionWidget::paintDissolve()
{
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
}

void TransitionWidget::paintGlitter()
{
    timer.stop();
}

void TransitionWidget::paintFly()
{
    timer.stop();
}

void TransitionWidget::paintPushUp()
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, picheight- split, 0, 0);
    painter.drawPixmap(shiftx, split, pixmap, 0, 0, 0, split);
}

void TransitionWidget::paintPushDown()
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split, picinit, 0, 0, 0, picheight- split);
    painter.drawPixmap(shiftx, shifty, pixmap, 0, picheight - split, 0, 0);
}

void TransitionWidget::paintPushLeft()
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, picwidth- split, 0, 0, 0);
    painter.drawPixmap(shiftx+split, shifty, pixmap, 0, 0, split, 0);
}

void TransitionWidget::paintPushRight()
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, picinit, 0, 0, picwidth- split, 0);
    painter.drawPixmap(shiftx, shifty, pixmap, picwidth - split, 0, 0, 0);
}

void TransitionWidget::paintCover()
{
    timer.stop();
}

void TransitionWidget::paintUncover()
{
    timer.stop();
}


void TransitionWidget::paintFade()
{
    painter.drawRect(shiftx, shifty, picwidth, picheight);
    painter.setOpacity(static_cast<double>(transition_duration - elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
}
