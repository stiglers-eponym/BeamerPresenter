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

PresentationSlide::PresentationSlide(PdfDoc const*const document, QWidget* parent) : DrawSlide(parent)
{
    doc = document;
    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&PresentationSlide::repaint));
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PresentationSlide::timeoutSignal);
    remainTimer.setSingleShot(true);
    connect(&remainTimer, &QTimer::timeout, this, &PresentationSlide::endAnimation);
}

PresentationSlide::~PresentationSlide()
{
    timer.stop();
    remainTimer.stop();
    timeoutTimer->stop();
    delete timeoutTimer;
}

void PresentationSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (remainTimer.isActive()) {
        (this->*paint)(painter);
        drawPointer(painter);
    }
    else {
        painter.drawPixmap(shiftx, shifty, pixmap);
        drawAnnotations(painter);
    }
}

void PresentationSlide::drawPointer(QPainter& painter)
{
    painter.setOpacity(1.);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);
    if (tool == Pointer) {
        painter.setPen(QPen(QColor(255,0,0,191), sizes[Pointer], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPoint(pointerPosition);
    }
    else if (tool == Torch && !pointerPosition.isNull()) {
        QPainterPath rectpath;
        rectpath.addRect(shiftx, shifty, pixmap.width(), pixmap.height());
        QPainterPath circpath;
        circpath.addEllipse(pointerPosition, sizes[Torch], sizes[Torch]);
        painter.fillPath(rectpath-circpath, QColor(0,0,0,48));
    }
}

void PresentationSlide::endAnimation()
{
    stopAnimation();
    repaint();
    emit endAnimationSignal();
    emit requestUpdateNotes(pageIndex);
}

void PresentationSlide::stopAnimation()
{
    timeoutTimer->stop();
    timer.stop();
    remainTimer.stop();
    if (!changes.isNull())
        changes = QPixmap();
    if (!picinit.isNull())
        picinit = QPixmap();
    if (!picfinal.isNull())
        picfinal = QPixmap();
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
    remainTimer.stop();
    transition_duration = -1;
    picinit = QPixmap();
    picfinal = QPixmap();
    changes = QPixmap();
}

void PresentationSlide::updateImages(int const oldPage)
{
    {
        picinit = QPixmap(size());
        picinit.fill(QColor(0,0,0,0));
        QPainter painter;
        painter.begin(&picinit);
        QString const& label = doc->getLabel(oldPage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPixmap(shiftx, shifty, getPixmap(oldPage));
        if (paths.contains(label)) {
            if (paths[label].contains(GreenPen)) {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(QColor(0,191,0), sizes[GreenPen]));
                for (QList<DrawPath>::const_iterator it=paths[label][GreenPen].cbegin(); it!=paths[label][GreenPen].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
            if (paths[label].contains(RedPen)) {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(QColor(255,0,0), sizes[RedPen]));
                for (QList<DrawPath>::const_iterator it=paths[label][RedPen].cbegin(); it!=paths[label][RedPen].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
            if (paths[label].contains(Highlighter)) {
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen(QColor(255,255,0), sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                for (QList<DrawPath>::const_iterator it=paths[label][Highlighter].cbegin(); it!=paths[label][Highlighter].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
        }
    }
    {
        picfinal = QPixmap(size());
        picfinal.fill(QColor(0,0,0,0));
        QPainter painter;
        painter.begin(&picfinal);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPixmap(shiftx, shifty, pixmap);
        if (paths.contains(page->label())) {
            if (paths[page->label()].contains(GreenPen)) {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(QColor(0,191,0), sizes[GreenPen]));
                for (QList<DrawPath>::const_iterator it=paths[page->label()][GreenPen].cbegin(); it!=paths[page->label()][GreenPen].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
            if (paths[page->label()].contains(RedPen)) {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(QColor(255,0,0), sizes[RedPen]));
                for (QList<DrawPath>::const_iterator it=paths[page->label()][RedPen].cbegin(); it!=paths[page->label()][RedPen].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
            if (paths[page->label()].contains(Highlighter)) {
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen(QColor(255,255,0), sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                for (QList<DrawPath>::const_iterator it=paths[page->label()][Highlighter].cbegin(); it!=paths[page->label()][Highlighter].cend(); it++)
                    painter.drawPolyline(it->data(), it->number());
            }
        }
    }
}

void PresentationSlide::animate(int const oldPageIndex) {
    if (transition_duration < 0 || oldPageIndex == pageIndex) {
        endAnimation();
        return;
    }
    timer.stop();
    remainTimer.stop();
    if (pixmap.isNull()) {
        transition_duration = 0;
        endAnimation();
        return;
    }
    picwidth = pixmap.width();
    picheight = pixmap.height();
    Poppler::PageTransition const* transition = page->transition();
    if (transition == nullptr || (duration>-1e-6 && duration < .05)) {
        transition_duration = 0;
        update();
        return;
    }
    if (oldPageIndex >= 0 && oldPageIndex > pageIndex)
        transition = doc->getPage(oldPageIndex)->transition();
    transition_duration = static_cast<int>(1000*transition->durationReal());
    if (transition_duration < 5) {
        endAnimation();
        return;
    }
    updateImages(oldPageIndex);
    remainTimer.setInterval(transition_duration-2);
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        {
        transition_duration = 0;
        endAnimation();
        return;
        }
    case Poppler::PageTransition::Split:
        qDebug () << "Transition split";
        if (transition->alignment() == Poppler::PageTransition::Horizontal) {
            if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward))
                paint = &PresentationSlide::paintSplitHI;
            else
                paint = &PresentationSlide::paintSplitHO;
        }
        else {
            if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward))
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
        if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward))
            paint = &PresentationSlide::paintBoxI;
        else
            paint = &PresentationSlide::paintBoxO;
        break;
    case Poppler::PageTransition::Wipe:
        {
        qDebug () << "Transition wipe" << transition->angle();
        int angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
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
        {
        qDebug() << "Transition fly" << transition->angle() << transition->direction() << transition->isRectangular() << transition->scale();
        QImage oldimg, newimg;
        if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward)) {
            oldimg = picinit.toImage();
            newimg = picfinal.toImage();
        }
        else {
            // The names are confusing, but remember that this is just the same as the "normal" fly transition with old and new image interchanged.
            newimg = picinit.toImage();
            oldimg = picfinal.toImage();
        }
        if (oldimg.size() != newimg.size())
            break;
        QImage alpha = QImage(size(), QImage::Format_Alpha8);
        alpha.fill(0);
        if (transition->isRectangular()) {
            // Find the smallest rectangle which includes all changes.
            int left=newimg.width(), right=0, top=newimg.height(), bottom=0;
            // get bottom
            for (int i=oldimg.height()-1; i>bottom; i--) {
                unsigned char const * const oldline = oldimg.constScanLine(i);
                unsigned char * const newline = newimg.scanLine(i);
                int j=0;
                for (; j<newimg.width(); j++) {
                    if (oldline[4*j] != newline[4*j] || oldline[4*j+1] != newline[4*j+1] || oldline[4*j+2] != newline[4*j+2]) {
                        bottom = i;
                        top = i;
                        left = j;
                        right = j;
                        break;
                    }
                }
            }
            // get top
            for (int i=0; i<top; i++) {
                unsigned char const * const oldline = oldimg.constScanLine(i);
                unsigned char * const newline = newimg.scanLine(i);
                int j=0;
                for (; j<newimg.width(); j++) {
                    if (oldline[4*j] != newline[4*j] || oldline[4*j+1] != newline[4*j+1] || oldline[4*j+2] != newline[4*j+2]) {
                        top = i;
                        if (left > j)
                            left = j;
                        else
                            right = j;
                        break;
                    }
                }
            }
            // get left and right
            for (int i=top; i<=bottom; i++) {
                unsigned char const * const oldline = oldimg.constScanLine(i);
                unsigned char * const newline = newimg.scanLine(i);
                int j=0;
                // get left
                for (; j<left; j++) {
                    if (oldline[4*j] != newline[4*j] || oldline[4*j+1] != newline[4*j+1] || oldline[4*j+2] != newline[4*j+2]) {
                        left = j;
                        break;
                    }
                }
                // get right
                for (j=right+1; j<newimg.width(); j++) {
                    if (oldline[4*j] != newline[4*j] || oldline[4*j+1] != newline[4*j+1] || oldline[4*j+2] != newline[4*j+2]) {
                        right = j;
                        break;
                    }
                }
            }
            for (int i=top; i<=bottom; i++) {
                unsigned char * const alphaline = alpha.scanLine(i);
                for (int j=left; j<=right; j++) {
                    alphaline[j] = 255;
                }
            }
        }
        else {
            double r, g, b, a;
            for (int i=0; i<oldimg.height(); i++) {
                unsigned char const * const oldline = oldimg.constScanLine(i);
                unsigned char * const newline = newimg.scanLine(i);
                unsigned char * const alphaline = alpha.scanLine(i);
                for (int j=0; j<oldimg.width(); j++) {
                    if (oldline[4*j] != newline[4*j] || oldline[4*j+1] != newline[4*j+1] || oldline[4*j+2] != newline[4*j+2]) {
                        // Do fancy transparency effects:
                        // Make the new pixels as transparent as possible while ensuring that the new page is given by adding the transparent new pixels to the old pixels.
                        // r := minimum alpha/255 required for the red channel
                        // g := minimum alpha/255 required for the green channel
                        // b := minimum alpha/255 required for the blue channel
                        r = oldline[4*j]   > newline[4*j]   ? 1. - static_cast<double>(newline[4*j]  )/oldline[4*j]   : static_cast<double>(newline[4*j]  -oldline[4*j]  )/(256-oldline[4*j]  );
                        g = oldline[4*j+1] > newline[4*j+1] ? 1. - static_cast<double>(newline[4*j+1])/oldline[4*j+1] : static_cast<double>(newline[4*j+1]-oldline[4*j+1])/(256-oldline[4*j+1]);
                        b = oldline[4*j+2] > newline[4*j+2] ? 1. - static_cast<double>(newline[4*j+2])/oldline[4*j+2] : static_cast<double>(newline[4*j+2]-oldline[4*j+2])/(256-oldline[4*j+2]);
                        // a := max(r, g, b) = minimum alpha/255 for the pixel
                        a = r > b ? (r > g ? r : g) : (b > g ? b : g);
                        alphaline[j]   = static_cast<unsigned char>(255.*a);
                        newline[4*j]   = static_cast<unsigned char>(.9999*(newline[4*j]   - oldline[4*j]  *(1.-a))/a);
                        newline[4*j+1] = static_cast<unsigned char>(.9999*(newline[4*j+1] - oldline[4*j+1]*(1.-a))/a);
                        newline[4*j+2] = static_cast<unsigned char>(.9999*(newline[4*j+2] - oldline[4*j+2]*(1.-a))/a);
                    }
                }
            }
        }
        newimg.setAlphaChannel(alpha);
        changes = QPixmap::fromImage(newimg);
        int angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
        if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward)) {
            if (transition->scale() < .999999) {
                transition_duration = static_cast<int>(transition_duration/(1.-transition->scale()));
                remainTimer.setInterval(static_cast<int>((1-transition->scale())*transition_duration)-2);
            }
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintFlyInRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintFlyInUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintFlyInLeft;
            else
                paint = &PresentationSlide::paintFlyInDown;
        }
        else {
            if (transition->scale() < .999999)
                virtual_transition_duration = static_cast<int>(transition_duration/(1.-transition->scale()));
            else
                virtual_transition_duration = transition_duration;
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintFlyOutRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintFlyOutUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintFlyOutLeft;
            else
                paint = &PresentationSlide::paintFlyOutDown;
        }
        }
        break;
    case Poppler::PageTransition::Push:
        {
        qDebug () << "Transition push" << transition->angle();
        int angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
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
        if (oldPageIndex < pageIndex) {
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintCoverRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintCoverUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintCoverLeft;
            else
                paint = &PresentationSlide::paintCoverDown;
        }
        else {
            angle = (angle + 180) % 360;
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintUncoverRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintUncoverUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintUncoverLeft;
            else
                paint = &PresentationSlide::paintUncoverDown;
        }
        }
        break;
    case Poppler::PageTransition::Uncover:
        {
        qDebug () << "Transition uncover" << transition->angle();
        int angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
        if (oldPageIndex < pageIndex) {
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintUncoverRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintUncoverUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintUncoverLeft;
            else
                paint = &PresentationSlide::paintUncoverDown;
        }
        else {
            if (angle < 45 || angle > 315)
                paint = &PresentationSlide::paintCoverRight;
            else if (angle < 135)
                paint = &PresentationSlide::paintCoverUp;
            else if (angle < 225)
                paint = &PresentationSlide::paintCoverLeft;
            else
                paint = &PresentationSlide::paintCoverDown;
        }
        }
        break;
    case Poppler::PageTransition::Fade:
        qDebug () << "Transition fade";
        paint = &PresentationSlide::paintFade;
        break;
    }
    emit requestUpdateNotes(pageIndex);
    remainTimer.start();
    timer.start(0);
}

void PresentationSlide::paintWipeUp(QPainter& painter)
{
    int const split = shifty + remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, split, picfinal, 0, split, -1, -1);
    painter.drawPixmap(0, 0, picinit, 0, 0, 0, split);
}

void PresentationSlide::paintWipeDown(QPainter& painter)
{
    int const split = shifty + (transition_duration - remainTimer.remainingTime())*picheight/transition_duration;
    painter.drawPixmap(0, split, picinit, 0, split, -1, -1);
    painter.drawPixmap(0, 0, picfinal, 0, 0, 0, split);
}

void PresentationSlide::paintWipeLeft(QPainter& painter)
{
    int const split = shiftx + remainTimer.remainingTime()*picwidth/transition_duration;
    painter.drawPixmap(split, 0, picfinal, split, 0, -1, -1);
    painter.drawPixmap(0, 0, picinit, 0, 0, split, -1);
}

void PresentationSlide::paintWipeRight(QPainter& painter)
{
    int const split = shiftx + (transition_duration - remainTimer.remainingTime())*picwidth/transition_duration;
    painter.drawPixmap(split, 0, picinit, split, 0, -1, -1);
    painter.drawPixmap(0, 0, picfinal, 0, 0, split, -1);
}

void PresentationSlide::paintBlindsV(QPainter& painter)
{
    int width = (picwidth*(transition_duration - remainTimer.remainingTime()))/(n_blinds*transition_duration);
    if (width == 0)
        width = 1;
    painter.drawPixmap(0, 0, picinit);
    int const n = this->width()*n_blinds/picwidth;
    for (int i=0; i<n; i++)
        painter.drawPixmap(i*picwidth/n_blinds, 0, picfinal, i*picwidth/n_blinds, 0, width, -1);
}

void PresentationSlide::paintBlindsH(QPainter& painter)
{
    int height = (picheight*(transition_duration - remainTimer.remainingTime()))/(n_blinds*transition_duration);
    if (height==0)
        height = 1;
    painter.drawPixmap(0, 0, picinit);
    int const n = this->height()*n_blinds/picheight;
    for (int i=0; i<n; i++)
        painter.drawPixmap(0, i*picheight/n_blinds, picfinal, 0, i*picheight/n_blinds, -1, height);
}

void PresentationSlide::paintBoxO(QPainter& painter)
{
    int const w = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    int const h = ((transition_duration - remainTimer.remainingTime())*picheight)/transition_duration;
    painter.drawPixmap(0, 0, picinit);
    painter.drawPixmap((width()-w)/2, (height()-h)/2, picfinal, (width()-w)/2, (height()-h)/2, w, h);
}

void PresentationSlide::paintBoxI(QPainter& painter)
{
    int const w = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    int const h = ((transition_duration - remainTimer.remainingTime())*picheight)/transition_duration;
    painter.drawPixmap(0, 0, picfinal);
    if (width() != w && height() != h)
        painter.drawPixmap(shiftx+w/2, shifty+h/2, picinit, shiftx+w/2, shifty+h/2, picwidth-w, picheight-h);
}

void PresentationSlide::paintSplitHO(QPainter& painter)
{
    int const h = ((transition_duration - remainTimer.remainingTime())*picheight)/transition_duration;
    painter.drawPixmap(0, 0, picinit, 0, 0, -1, (height()-h)/2+1);
    painter.drawPixmap(0, (height()+h)/2, picinit,  0, (height()+h)/2, -1, (height()-h)/2+1);
    painter.drawPixmap(0, (height()-h)/2, picfinal, 0, (height()-h)/2, -1, h);
}

void PresentationSlide::paintSplitVO(QPainter& painter)
{
    int const w = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    painter.drawPixmap(0, 0, picinit, 0, 0, (width()-w)/2+1, -1);
    painter.drawPixmap((width()+w)/2, 0, picinit,  (width()+w)/2, 0, (width()-w)/2+1, -1);
    painter.drawPixmap((width()-w)/2, 0, picfinal, (width()-w)/2, 0, w, -1);
}

void PresentationSlide::paintSplitHI(QPainter& painter)
{
    int const h = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, 0, picfinal, 0, 0, -1, (height()-h)/2+1);
    painter.drawPixmap(0, (height()+h)/2, picfinal, 0, (height()+h)/2, -1, (height()-h)/2+1);
    painter.drawPixmap(0, (height()-h)/2, picinit,  0, (height()-h)/2, -1, h);
}

void PresentationSlide::paintSplitVI(QPainter& painter)
{
    int const w = remainTimer.remainingTime()*picwidth/transition_duration;
    painter.drawPixmap(0, 0, picfinal, 0, 0, (width()-w)/2+1, -1);
    painter.drawPixmap((width()+w)/2, 0, picfinal, (width()+w)/2, 0, (width()-w)/2+1, -1);
    painter.drawPixmap((width()-w)/2, 0, picinit,  (width()-w)/2, 0, w, -1);
}

void PresentationSlide::paintDissolve(QPainter& painter)
{
    painter.setOpacity(static_cast<double>(remainTimer.remainingTime())/transition_duration);
    painter.drawPixmap(0, 0, picinit);
    painter.setOpacity(static_cast<double>((transition_duration - remainTimer.remainingTime()))/transition_duration);
    painter.drawPixmap(0, 0, picfinal);
}

int const glitter[] = {8, 118, 37, 106, 155, 67, 113, 92, 140, 163, 57, 82, 14, 72, 12, 54, 43, 139, 91, 156, 108, 149, 98, 138, 32, 166, 58, 159, 60, 39, 62, 131, 40, 147, 145, 46, 153, 110, 93, 132, 85, 3, 78, 18, 87, 119, 165, 31, 109, 121, 28, 101, 127, 50, 96, 0, 95, 164, 100, 64, 2, 23, 105, 158, 49, 29, 52, 35, 13, 146, 154, 111, 45, 102, 55, 42, 15, 79, 1, 114, 27, 122, 104, 84, 53, 73, 126, 143, 157, 16, 59, 123, 74, 51, 24, 88, 11, 161, 135, 47, 4, 152, 86, 34, 130, 66, 17, 25, 89, 38, 160, 65, 112, 26, 63, 148, 48, 70, 22, 61, 33, 9, 141, 128, 162, 41, 19, 36, 77, 129, 68, 107, 20, 69, 81, 125, 94, 124, 90, 120, 133, 7, 75, 71, 116, 137, 150, 21, 30, 103, 5, 134, 115, 83, 10, 117, 144, 97, 142, 76, 151, 99, 56, 44, 6, 80, 136};
int const nglitter = 167;
int const glitterpixel = 30;

void PresentationSlide::paintGlitter(QPainter& painter)
{
    int const n = width()*height()/glitterpixel;
    int const steps = (nglitter*(transition_duration - remainTimer.remainingTime()))/transition_duration;
    int const w = width()/glitterpixel;
    painter.drawPixmap(0, 0, picinit);
    for (int j=0; j<steps; j++) {
        for (int i=glitter[j]%nglitter; i<n; i+=nglitter) {
            painter.drawPixmap(glitterpixel*(i%w), glitterpixel*(i/w), picfinal, glitterpixel*(i%w), glitterpixel*(i/w), glitterpixel, glitterpixel);
        }
    }
}

void PresentationSlide::paintFlyInUp(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*height())/transition_duration;
    painter.drawPixmap(0, 0, picinit);
    if (split != 0)
        painter.drawPixmap(0, height()-split, changes, 0, 0, -1, split);
}

void PresentationSlide::paintFlyInDown(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*height()/transition_duration;
    painter.drawPixmap(0, 0, picinit);
    if (split != height())
        painter.drawPixmap(0, 0, changes, 0, split, -1, -1);
}

void PresentationSlide::paintFlyInLeft(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*width()/transition_duration;
    painter.drawPixmap(0, 0, picinit);
    if (split != width())
        painter.drawPixmap(split, 0, changes, 0, 0, width(), -1);
}

void PresentationSlide::paintFlyInRight(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*width()/transition_duration;
    painter.drawPixmap(0, 0, picinit);
    if (split != width())
        painter.drawPixmap(0, 0, changes, split, 0, -1, -1);
}

void PresentationSlide::paintFlyOutUp(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*height())/virtual_transition_duration;
    painter.drawPixmap(0, 0, picfinal);
    if (split != 0)
        painter.drawPixmap(0, 0, changes, 0, split, -1, -1);
}

void PresentationSlide::paintFlyOutDown(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*height())/virtual_transition_duration;
    painter.drawPixmap(0, 0, picfinal);
    if (split != 0)
        painter.drawPixmap(0, split, changes, 0, 0, -1, height()-split);
}

void PresentationSlide::paintFlyOutRight(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*width())/virtual_transition_duration;
    painter.drawPixmap(0, 0, picfinal);
    if (split != 0)
        painter.drawPixmap(split, 0, changes, 0, 0, width()-split, -1);
}

void PresentationSlide::paintFlyOutLeft(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*width())/virtual_transition_duration;
    painter.drawPixmap(0, 0, picfinal);
    if (split != 0)
        painter.drawPixmap(0, 0, changes, split, 0, -1, -1);
}

void PresentationSlide::paintPushUp(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, 0, picinit, 0, picheight-split, 0, split+shifty+1);
    if (split != picheight)
        painter.drawPixmap(0, split, picfinal, 0, 0, -1, height()-split);
}

void PresentationSlide::paintPushDown(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, picheight-split, picinit, 0, 0, -1, -1);
    if (split != picheight)
        painter.drawPixmap(0, 0, picfinal, 0, split, -1, height()-split);
}

void PresentationSlide::paintPushLeft(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picwidth/transition_duration;
    painter.drawPixmap(0, 0, picinit, picwidth-split, 0, split+shiftx+1, -1);
    if (split != picwidth)
        painter.drawPixmap(split, 0, picfinal, 0, 0, width()-split, -1);
}

void PresentationSlide::paintPushRight(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picwidth/transition_duration;
    painter.drawPixmap(picwidth-split, 0, picinit, 0, 0, -1, -1);
    if (split != picwidth)
        painter.drawPixmap(0, 0, picfinal, split, 0, width()-split, -1);
}

void PresentationSlide::paintCoverUp(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, 0, picinit, 0, 0, -1, height()-picheight+split);
    if (split != picheight)
        painter.drawPixmap(0, split, picfinal, 0, 0, -1, -1);
}

void PresentationSlide::paintCoverDown(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, picheight-split, picinit, 0, picheight-split, -1, height()-picheight+split);
    if (split != picheight)
        painter.drawPixmap(0, 0, picfinal, 0, split, -1, height()-split);
}

void PresentationSlide::paintCoverLeft(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    painter.drawPixmap(0, 0, picinit, 0, 0, width()-split, -1);
    if (split != 0)
        painter.drawPixmap(picwidth-split, 0, picfinal, 0, 0, -1, -1);
}

void PresentationSlide::paintCoverRight(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    painter.drawPixmap(split, 0, picinit, split, 0, -1, -1);
    if (split != 0)
        painter.drawPixmap(0, 0, picfinal, picwidth-split, 0, -1, -1);
}

void PresentationSlide::paintUncoverDown(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*picheight)/transition_duration;
    painter.drawPixmap(0, 0, picfinal, 0, 0, -1, height()-picheight+split);
    if (split != 0)
        painter.drawPixmap(0, split, picinit, 0, 0, -1, height()-split);
}

void PresentationSlide::paintUncoverUp(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picheight/transition_duration;
    painter.drawPixmap(0, split, picfinal, 0, split, -1, -1);
    if (split != picheight)
        painter.drawPixmap(0, 0, picinit, 0, picheight-split, -1, -1);
}

void PresentationSlide::paintUncoverRight(QPainter& painter)
{
    int const split = ((transition_duration - remainTimer.remainingTime())*picwidth)/transition_duration;
    painter.drawPixmap(0, 0, picfinal, 0, 0, width()-picwidth+split, -1);
    if (split != 0)
        painter.drawPixmap(split, 0, picinit, 0, 0, width()-split, -1);
}

void PresentationSlide::paintUncoverLeft(QPainter& painter)
{
    int const split = remainTimer.remainingTime()*picwidth/transition_duration;
    painter.drawPixmap(split, 0, picfinal, split, 0, -1, -1);
    if (split != picwidth)
        painter.drawPixmap(0, 0, picinit, picwidth-split, 0, -1, -1);
}

void PresentationSlide::paintFade(QPainter& painter)
{
    painter.drawPixmap(0, 0, picinit);
    painter.setOpacity(static_cast<double>((transition_duration - remainTimer.remainingTime()))/transition_duration);
    painter.drawPixmap(0, 0, picfinal);
}
