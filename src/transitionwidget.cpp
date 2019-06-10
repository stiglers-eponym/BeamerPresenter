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

TransitionWidget::TransitionWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    connect(&timer, &QTimer::timeout, this, &TransitionWidget::animationStep);
}

void TransitionWidget::shiftImages(const QPixmap pic)
{
     picinit=picfinal;
     picfinal=pic;
     picwidth = picfinal.width();
     picheight = picfinal.height();
}

void TransitionWidget::animationStep()
{
    elapsed += dt;
    if (elapsed >= duration) {
        timer.stop();
        hide();
        return;
    }
    painter.begin(this);
    (this->*paint)();
    painter.end();
    qobject_cast<QWidget*>(parent())->repaint();
}

void TransitionWidget::animate(Poppler::PageTransition const* transition) {
    // TODO: Test and implement more transitions
    timer.stop();
    if (picinit.isNull() || picfinal.isNull()) {
        hide();
        return;
    }
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        hide();
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
    duration = static_cast<int>(1000*transition->durationReal());
    elapsed = 0;
    painter.begin(this);
    painter.drawPixmap(0, 0, picinit);
    painter.end();
    show();
    timer.start(dt);
}

void TransitionWidget::paintWipeUp()
{
    int const split = picheight - elapsed*picheight/duration;
    painter.drawPixmap(0, split, picfinal, 0, split, 0, split + dt*picheight/duration + 1);
}

void TransitionWidget::paintWipeDown()
{
    int const split = elapsed*picheight/duration;
    int const start = split - dt*picheight/duration - 1;
    painter.drawPixmap(0, start, picfinal, 0, start, 0, split);
}

void TransitionWidget::paintWipeLeft()
{
    int const split = picwidth - elapsed*picwidth/duration;
    painter.drawPixmap(split, 0, picfinal, split, 0, split + dt*picwidth/duration + 1, 0);
}

void TransitionWidget::paintWipeRight()
{
    int const split = elapsed*picwidth/duration;
    int const start = split - dt*picwidth/duration - 1;
    painter.drawPixmap(start, 0, picfinal, start, 0, split, 0);
}

void TransitionWidget::paintBlindsV()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picwidth)/(n_blinds*duration) - 1 : 0;
    int width = (picwidth*elapsed)/(n_blinds*duration) - shift;
    if (width==0)
        width = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(i*picwidth/n_blinds+shift, 0, picfinal, i*picwidth/n_blinds+shift, 0, width, -1);
}

void TransitionWidget::paintBlindsH()
{
    int const shift = elapsed>dt ? ((elapsed-dt)*picheight)/(n_blinds*duration) - 1 : 0;
    int height = (picheight*elapsed)/(n_blinds*duration)-shift;
    if (height==0)
        height = 1;
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(0, i*picheight/n_blinds+shift, picfinal, 0, i*picheight/n_blinds+shift, -1, height);
}

void TransitionWidget::paintBoxO()
{
    int const width = (elapsed*picwidth)/duration;
    int const height = (elapsed*picheight)/duration;
    painter.drawPixmap((picwidth-width)/2, (picheight-height)/2, picfinal, (picwidth-width)/2, (picheight-height)/2, width, height);

    // The following code should be more efficient, but it caused rendering errors when I tried it.
    //int const width = (elapsed*picwidth)/duration;
    //int const height = (elapsed*picheight)/duration;
    //int const dw = (dt*picwidth)/(2*duration);
    //int const dh = (dt*picheight)/(2*duration);
    //painter.drawPixmap((picwidth-width)/2, (picheight-height)/2, picfinal, (picwidth-width)/2, (picheight-height)/2, width, dh);
    //painter.drawPixmap((picwidth-width)/2, (picheight-height)/2+dh, picfinal, (picwidth-width)/2, (picheight-height)/2, dw, height-2*dh);
    //painter.drawPixmap((picwidth-width)/2, (picheight+height)/2-dh, picfinal, (picwidth-width)/2, (picheight+height)/2-dh, width, dh);
    //painter.drawPixmap((picwidth+width)/2-dw, (picheight-height)/2, picfinal, (picwidth+width)/2-dw, (picheight-height)/2, dw, height-2*dh);
}

void TransitionWidget::paintBoxI()
{
    int const width = (elapsed*picwidth)/duration;
    int const height = (elapsed*picheight)/duration;
    int const dw = (dt*picwidth)/(2*duration)+1;
    int const dh = (dt*picheight)/(2*duration)+1;
    painter.drawPixmap(width/2-dw, height/2-dh, picfinal, width/2-dw, height/2-dh, picwidth-width+2*dw, dh);
    painter.drawPixmap(width/2-dw, height/2, picfinal, width/2-dw, height/2, dw, picheight-height+1);
    painter.drawPixmap(width/2-dw, picheight-height/2, picfinal, width/2-dw, picheight-height/2, picwidth-width+2*dw, dh);
    painter.drawPixmap(picwidth-width/2, height/2, picfinal, picwidth-width/2, height/2, dw, picheight-height+1);
}

void TransitionWidget::paintSplitHO()
{
    int const height = (elapsed*picheight)/duration;
    int const dh = (dt*picheight)/duration + 1;
    painter.drawPixmap(0, (picheight-height)/2, picfinal, 0, (picheight-height)/2, -1, dh);
    painter.drawPixmap(0, (picheight+height)/2-dh, picfinal, 0, (picheight+height)/2-dh, -1, dh);
}

void TransitionWidget::paintSplitVO()
{
    int const width = (elapsed*picwidth)/duration;
    int const dw = (dt*picheight)/duration + 1;
    painter.drawPixmap((picwidth-width)/2, 0, picfinal, (picwidth-width)/2, 0, dw, -1);
    painter.drawPixmap((picwidth+width)/2-dw, 0, picfinal, (picwidth+width)/2-dw, 0, dw, -1);
}

void TransitionWidget::paintSplitHI()
{
    int const height = (elapsed*picheight)/duration;
    int const dh = (dt*picheight)/duration + 1;
    painter.drawPixmap(0, height/2-dh, picfinal, 0, height/2-dh, -1, dh);
    painter.drawPixmap(0, picheight-height/2, picfinal, 0, picheight-height/2, -1, dh);
}

void TransitionWidget::paintSplitVI()
{
    int const width = (elapsed*picwidth)/duration;
    int const dw = (dt*picheight)/duration + 1;
    painter.drawPixmap(width/2-dw, 0, picfinal, width/2-dw, 0, dw, -1);
    painter.drawPixmap(picwidth-width/2, 0, picfinal, picwidth-width/2, 0, dw, -1);
}

void TransitionWidget::paintDissolve()
{
    painter.drawPixmap(0, 0, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/duration);
    painter.drawPixmap(0, 0, picfinal);
}

void TransitionWidget::paintGlitter()
{
    timer.stop();
    hide();
    return;
}

void TransitionWidget::paintFly()
{
    timer.stop();
    hide();
    return;
}

void TransitionWidget::paintPushUp()
{
    int const split = picheight - elapsed*picheight/duration;
    painter.drawPixmap(0, 0, picinit, 0, picheight- split, 0, 0);
    painter.drawPixmap(0, split, picfinal, 0, 0, 0, split);
}

void TransitionWidget::paintPushDown()
{
    int const split = elapsed*picheight/duration;
    painter.drawPixmap(0, split, picinit, 0, 0, 0, picheight- split);
    painter.drawPixmap(0, 0, picfinal, 0, picheight - split, 0, 0);
}

void TransitionWidget::paintPushLeft()
{
    int const split = picwidth - elapsed*picwidth/duration;
    painter.drawPixmap(0, 0, picinit, picwidth- split, 0, 0, 0);
    painter.drawPixmap(split, 0, picfinal, 0, 0, split, 0);
}

void TransitionWidget::paintPushRight()
{
    int const split = elapsed*picwidth/duration;
    painter.drawPixmap(split, 0, picinit, 0, 0, picwidth- split, 0);
    painter.drawPixmap(0, 0, picfinal, picwidth - split, 0, 0, 0);
}

void TransitionWidget::paintCover()
{
    timer.stop();
    hide();
    return;
}

void TransitionWidget::paintUncover()
{
    timer.stop();
    hide();
    return;
}


void TransitionWidget::paintFade()
{
    painter.drawRect(0, 0, picwidth, picheight);
    painter.setOpacity(static_cast<double>(duration - elapsed)/duration);
    painter.drawPixmap(0, 0, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/duration);
    painter.drawPixmap(0, 0, picfinal);
}
