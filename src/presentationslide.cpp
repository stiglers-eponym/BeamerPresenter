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

#include "presentationslide.h"

PresentationSlide::PresentationSlide(QWidget* parent) : DrawSlide(parent)
{
    connect(&timer, &QTimer::timeout, this, &PresentationSlide::updateFromTimer);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PresentationSlide::timeoutSignal);
}

PresentationSlide::~PresentationSlide()
{
    timer.stop();
    timeoutTimer->stop();
    delete timeoutTimer;
}

void PresentationSlide::updateFromTimer()
{
    elapsed += dt;
    update();
}

void PresentationSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (elapsed < 0) {
        painter.drawPixmap(shiftx, shifty, pixmap);
        drawAnnotations(painter);
    }
    else if (elapsed >= transition_duration) {
        timer.stop();
        painter.drawPixmap(shiftx, shifty, pixmap);
        picinit = pixmap;
        drawAnnotations(painter);
    }
    else
        (this->*paint)(painter);
}

void PresentationSlide::endAnimation()
{
    timeoutTimer->stop();
    if (timer.isActive()) {
        elapsed = transition_duration;
        picinit = pixmap;
    }
}

void PresentationSlide::setDuration()
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

void PresentationSlide::togglePointerVisibility()
{
    if (pointer_visible) {
        pointer_visible = false;
        setCursor(Qt::BlankCursor);
        if (tool != Pointer)
            setMouseTracking(false);
    }
    else {
        pointer_visible = true;
        setCursor(Qt::ArrowCursor);
        setMouseTracking(true);
    }
}

void PresentationSlide::disableTransitions()
{
    timer.stop();
    elapsed = -1;
    transition_duration = 0;
    picinit = QPixmap();
}

void PresentationSlide::animate() {
    // TODO: Test and implement more transitions

    // elapsed < 0 disables slide transitions.
    if (elapsed < 0) {
        update();
        return;
    }
    elapsed = 0;
    timer.stop();
    if (pixmap.isNull()) {
        transition_duration = 0;
        return;
    }
    picwidth = pixmap.width();
    picheight = pixmap.height();
    Poppler::PageTransition const* transition = page->transition();
    if (transition == nullptr || picinit.isNull() || (duration>-1e-6 && 250*duration < dt)) {
        transition_duration = 0;
        update();
        return;
    }
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        {
        transition_duration = 0;
        update();
        return;
        }
    case Poppler::PageTransition::Split:
        qDebug () << "Transition split";
        if (transition->alignment() == Poppler::PageTransition::Horizontal) {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &PresentationSlide::paintSplitHI;
            else
                paint = &PresentationSlide::paintSplitHO;
        }
        else {
            if (transition->direction() == Poppler::PageTransition::Inward)
                paint = &PresentationSlide::paintSplitVI;
            else
                paint = &PresentationSlide::paintSplitVO;
        }
        break;
    case Poppler::PageTransition::Blinds:
        qDebug () << "Transition blinds";
        if (transition->alignment() == Poppler::PageTransition::Horizontal)
            paint = &PresentationSlide::paintBlindsH;
        else
            paint = &PresentationSlide::paintBlindsV;
        break;
    case Poppler::PageTransition::Box:
        qDebug () << "Transition box";
        if (transition->direction() == Poppler::PageTransition::Inward)
            paint = &PresentationSlide::paintBoxI;
        else
            paint = &PresentationSlide::paintBoxO;
        break;
    case Poppler::PageTransition::Wipe:
        {
        qDebug () << "Transition wipe" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationSlide::paintWipeRight;
        else if (angle < 135)
            paint = &PresentationSlide::paintWipeUp;
        else if (angle < 225)
            paint = &PresentationSlide::paintWipeLeft;
        else
            paint = &PresentationSlide::paintWipeDown;
        }
        break;
    case Poppler::PageTransition::Dissolve:
        qDebug () << "Transition dissolve";
        paint = &PresentationSlide::paintDissolve;
        break;
    case Poppler::PageTransition::Glitter:
        qDebug () << "Transition glitter";
        paint = &PresentationSlide::paintGlitter;
        break;
    case Poppler::PageTransition::Fly:
        qWarning () << "Unsupported slide transition fly";
        qInfo() << "If you know how this slide transition should look like, please tell me:";
        qInfo() << "    https://github.com/stiglers-eponym/BeamerPresenter/issues";
        paint = &PresentationSlide::paintFly;
        break;
    case Poppler::PageTransition::Push:
        {
        qDebug () << "Transition push" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationSlide::paintPushRight;
        else if (angle < 135)
            paint = &PresentationSlide::paintPushUp;
        else if (angle < 225)
            paint = &PresentationSlide::paintPushLeft;
        else
            paint = &PresentationSlide::paintPushDown;
        }
        break;
    case Poppler::PageTransition::Cover:
        {
        qDebug () << "Transition cover" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationSlide::paintCoverRight;
        else if (angle < 135)
            paint = &PresentationSlide::paintCoverUp;
        else if (angle < 225)
            paint = &PresentationSlide::paintCoverLeft;
        else
            paint = &PresentationSlide::paintCoverDown;
        }
        break;
    case Poppler::PageTransition::Uncover:
        {
        qDebug () << "Transition uncover" << transition->angle();
        int angle = (360 + transition->angle()) % 360;
        if (angle < 45 || angle > 315)
            paint = &PresentationSlide::paintUncoverRight;
        else if (angle < 135)
            paint = &PresentationSlide::paintUncoverUp;
        else if (angle < 225)
            paint = &PresentationSlide::paintUncoverLeft;
        else
            paint = &PresentationSlide::paintUncoverDown;
        }

        break;
    case Poppler::PageTransition::Fade:
        qDebug () << "Transition fade";
        paint = &PresentationSlide::paintFade;
        break;
    }
    elapsed += dt/2;
    transition_duration = static_cast<int>(1000*transition->durationReal());
    timer.start(dt);
}

void PresentationSlide::paintWipeUp(QPainter& painter)
{
    int const split = picheight - elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, -1, -1);
    DrawTool const realtool = tool;
    if (tool != Pointer)
        tool = None;
    drawAnnotations(painter);
    tool = realtool;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, 0, split);
}

void PresentationSlide::paintWipeDown(QPainter& painter)
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, split+shifty, pixmap, 0, split, -1, -1);
    DrawTool const realtool = tool;
    if (tool != Pointer)
        tool = None;
    drawAnnotations(painter);
    tool = realtool;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, 0, split);
}

void PresentationSlide::paintWipeLeft(QPainter& painter)
{
    int const split = picwidth - elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, -1, -1);
    DrawTool const realtool = tool;
    if (tool != Pointer)
        tool = None;
    drawAnnotations(painter);
    tool = realtool;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, split, -1);
}

void PresentationSlide::paintWipeRight(QPainter& painter)
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, pixmap, split, 0, -1, -1);
    DrawTool const realtool = tool;
    if (tool != Pointer)
        tool = None;
    drawAnnotations(painter);
    tool = realtool;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, split, -1);
}

void PresentationSlide::paintBlindsV(QPainter& painter)
{
    int width = (picwidth*elapsed)/(n_blinds*transition_duration);
    if (width==0)
        width = 1;
    painter.drawPixmap(shiftx, shifty, picinit);
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx+i*picwidth/n_blinds, shifty, pixmap, i*picwidth/n_blinds, 0, width, -1);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintBlindsH(QPainter& painter)
{
    int height = (picheight*elapsed)/(n_blinds*transition_duration);
    if (height==0)
        height = 1;
    painter.drawPixmap(shiftx, shifty, picinit);
    for (int i=0; i<n_blinds; i++)
        painter.drawPixmap(shiftx, i*picheight/n_blinds+shifty, pixmap, 0, i*picheight/n_blinds, -1, height);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintBoxO(QPainter& painter)
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty+(picheight-height)/2, pixmap, (picwidth-width)/2, (picheight-height)/2, width, height);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintBoxI(QPainter& painter)
{
    int const width = (elapsed*picwidth)/transition_duration;
    int const height = (elapsed*picheight)/transition_duration;
    if (picwidth == width || picheight == height) {
        painter.drawPixmap(shiftx, shifty, pixmap);
        drawAnnotations(painter);
    }
    else {
        painter.drawPixmap(shiftx, shifty, pixmap);
        if (!isOverlay)
            drawAnnotations(painter);
        painter.drawPixmap(shiftx+width/2, shifty+height/2, picinit, width/2, height/2, picwidth-width, picheight-height);
        if (isOverlay)
            drawAnnotations(painter);
    }
}

void PresentationSlide::paintSplitHO(QPainter& painter)
{
    int const height = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight+height)/2, picinit, 0, (picheight+height)/2, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight-height)/2, pixmap, 0, (picheight-height)/2, -1, height);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintSplitVO(QPainter& painter)
{
    int const width = (elapsed*picwidth)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth+width)/2, shifty, picinit, (picwidth+width)/2, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, pixmap, (picwidth-width)/2, 0, width, -1);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintSplitHI(QPainter& painter)
{
    int const height = picheight - (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight+height)/2, pixmap, 0, (picheight+height)/2, -1, (picheight-height)/2+1);
    painter.drawPixmap(shiftx, shifty+(picheight-height)/2, picinit, 0, (picheight-height)/2, -1, height);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintSplitVI(QPainter& painter)
{
    int const width = picwidth - (elapsed*picwidth)/transition_duration;
    painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth+width)/2, shifty, pixmap, (picwidth+width)/2, 0, (picwidth-width)/2+1, -1);
    painter.drawPixmap(shiftx+(picwidth-width)/2, shifty, picinit, (picwidth-width)/2, 0, width, -1);
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintDissolve(QPainter& painter)
{
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
    if (isOverlay) {
        painter.setOpacity(1.);
        drawAnnotations(painter);
    }
}

int const glitter[] = {8, 118, 37, 106, 155, 67, 113, 92, 140, 163, 57, 82, 14, 72, 12, 54, 43, 139, 91, 156, 108, 149, 98, 138, 32, 166, 58, 159, 60, 39, 62, 131, 40, 147, 145, 46, 153, 110, 93, 132, 85, 3, 78, 18, 87, 119, 165, 31, 109, 121, 28, 101, 127, 50, 96, 0, 95, 164, 100, 64, 2, 23, 105, 158, 49, 29, 52, 35, 13, 146, 154, 111, 45, 102, 55, 42, 15, 79, 1, 114, 27, 122, 104, 84, 53, 73, 126, 143, 157, 16, 59, 123, 74, 51, 24, 88, 11, 161, 135, 47, 4, 152, 86, 34, 130, 66, 17, 25, 89, 38, 160, 65, 112, 26, 63, 148, 48, 70, 22, 61, 33, 9, 141, 128, 162, 41, 19, 36, 77, 129, 68, 107, 20, 69, 81, 125, 94, 124, 90, 120, 133, 7, 75, 71, 116, 137, 150, 21, 30, 103, 5, 134, 115, 83, 10, 117, 144, 97, 142, 76, 151, 99, 56, 44, 6, 80, 136};
int const nglitter = 167;
int const glitterpixel = 30;

void PresentationSlide::paintGlitter(QPainter& painter)
{
    int const n = picwidth*picheight/glitterpixel;
    int const steps = (nglitter*elapsed)/transition_duration;
    int const w = picwidth/glitterpixel;
    painter.drawPixmap(shiftx, shifty, picinit);
    for (int j=0; j<steps; j++) {
        for (int i=glitter[j]%nglitter; i<n; i+=nglitter) {
            painter.drawPixmap(shiftx+glitterpixel*(i%w), shifty+glitterpixel*(i/w), pixmap, glitterpixel*(i%w), glitterpixel*(i/w), glitterpixel, glitterpixel);
        }
    }
    if (isOverlay)
        drawAnnotations(painter);
}

void PresentationSlide::paintFly(QPainter& painter)
{
    // TODO
    // How should this transition look like?
    painter.drawPixmap(shiftx, shifty, pixmap);
    if (isOverlay)
        drawAnnotations(painter);
    transition_duration = 0;
}

void PresentationSlide::paintPushUp(QPainter& painter)
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, split, 0, 0);
    if (split!=0)
        painter.drawPixmap(shiftx, shifty+picheight-split, pixmap, 0, 0, -1, split);
}

void PresentationSlide::paintPushDown(QPainter& painter)
{
    int const split = elapsed*picheight/transition_duration;
    painter.drawPixmap(shiftx, shifty+split, picinit, 0, 0, -1, picheight-split);
    if (split!=0)
        painter.drawPixmap(shiftx, shifty, pixmap, 0, picheight-split, -1, -1);
}

void PresentationSlide::paintPushLeft(QPainter& painter)
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, split, 0, -1, -1);
    if (split!=0)
        painter.drawPixmap(shiftx+picwidth-split, shifty, pixmap, 0, 0, split, -1);
}

void PresentationSlide::paintPushRight(QPainter& painter)
{
    int const split = elapsed*picwidth/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, picinit, 0, 0, picwidth-split, -1);
    if (split!=0)
        painter.drawPixmap(shiftx, shifty, pixmap, picwidth-split, 0, -1, -1);
}

void PresentationSlide::paintCoverUp(QPainter& painter)
{
    int const split = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, -1, picheight-split);
    if (split != 0)
        painter.drawPixmap(shiftx, shifty+picheight-split, pixmap, 0, 0, -1, split);
}

void PresentationSlide::paintCoverDown(QPainter& painter)
{
    int const split = (elapsed*picheight)/transition_duration;
    painter.drawPixmap(shiftx, shifty+split, picinit, 0, split, -1, -1);
    if (split != 0)
        painter.drawPixmap(shiftx, shifty, pixmap, 0, picheight-split, -1, -1);
}

void PresentationSlide::paintCoverLeft(QPainter& painter)
{
    int const split = (elapsed*picwidth)/transition_duration;
    painter.drawPixmap(shiftx, shifty, picinit, 0, 0, picwidth-split, -1);
    if (split != 0)
        painter.drawPixmap(shiftx+picwidth-split, shifty, pixmap, 0, 0, picwidth, -1);
}

void PresentationSlide::paintCoverRight(QPainter& painter)
{
    int const split = (elapsed*picwidth)/transition_duration;
    painter.drawPixmap(shiftx+split, shifty, picinit, split, 0, -1, -1);
    if (split != 0)
        painter.drawPixmap(shiftx, shifty, pixmap, picwidth-split, 0, -1, -1);
}

void PresentationSlide::paintUncoverDown(QPainter& painter)
{
    int const split = (elapsed*picheight)/transition_duration;
    if (split != 0)
        painter.drawPixmap(shiftx, shifty+split, picinit, 0, 0, -1, picheight-split);
    painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, -1, split);
}

void PresentationSlide::paintUncoverUp(QPainter& painter)
{
    int const split = (elapsed*picheight)/transition_duration;
    if (split != 0)
        painter.drawPixmap(shiftx, shifty, picinit, 0, split, -1, -1);
    painter.drawPixmap(shiftx, shifty+picheight-split, pixmap, 0, picheight-split, -1, -1);
}

void PresentationSlide::paintUncoverRight(QPainter& painter)
{
    int const split = (elapsed*picwidth)/transition_duration;
    if (split != 0)
        painter.drawPixmap(shiftx+split, shifty, picinit, 0, 0, picwidth-split, -1);
    painter.drawPixmap(shiftx, shifty, pixmap, 0, 0, split, -1);
}

void PresentationSlide::paintUncoverLeft(QPainter& painter)
{
    int const split = (elapsed*picwidth)/transition_duration;
    if (split != 0)
        painter.drawPixmap(shiftx, shifty, picinit, split, 0, -1, -1);
    painter.drawPixmap(shiftx+picwidth-split, shifty, pixmap, picwidth-split, 0, -1, -1);
}

void PresentationSlide::paintFade(QPainter& painter)
{
    painter.setOpacity(static_cast<double>(transition_duration - elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, picinit);
    painter.setOpacity(static_cast<double>(elapsed)/transition_duration);
    painter.drawPixmap(shiftx, shifty, pixmap);
    if (isOverlay) {
        painter.setOpacity(1.);
        drawAnnotations(painter);
    }
}
