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

PresentationSlide::PresentationSlide(PdfDoc const*const document, QWidget* parent) :
    DrawSlide(document, 0, parent)
{
    seed = static_cast<unsigned int>(std::hash<std::string>{}(doc->getPath().split('/').last().toStdString()));
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
    if (glitter != nullptr) {
        delete[] glitter;
        glitter = nullptr;
    }
}

void PresentationSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (remainTimer.isActive() && remainTimer.interval()>0 && this->paint != nullptr) {
        (this->*paint)(painter);
        drawPointer(painter);
    }
    else {
        if (pixpaths.isNull() || end_cache < 1)
            painter.drawPixmap(shiftx, shifty, pixmap);
        else
            painter.drawPixmap(0, 0, pixpaths);
        drawAnnotations(painter);
    }
}

void PresentationSlide::drawPointer(QPainter& painter)
{
    painter.setOpacity(1.);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);
    if (tool.tool == Pointer) {
        painter.setPen(QPen(QColor(255,0,0,191), sizes[Pointer], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPoint(pointerPosition);
    }
    else if (tool.tool == Torch && !pointerPosition.isNull()) {
        QPainterPath rectpath;
        rectpath.addRect(shiftx, shifty, pixmap.width(), pixmap.height());
        QPainterPath circpath;
        circpath.addEllipse(pointerPosition, sizes[Torch], sizes[Torch]);
        painter.fillPath(rectpath-circpath, QColor(0,0,0,48));
    }
}

void PresentationSlide::undoPath()
{
    if (!paths[page->label()].isEmpty()) {
        undonePaths.append(paths[page->label()].takeLast());
        end_cache = -1;
        pixpaths = QPixmap();
        repaint();
        emit pathsChangedQuick(page->label(), paths[page->label()], shiftx, shifty, resolution);
    }
}

void PresentationSlide::redoPath()
{
    if (!undonePaths.isEmpty()) {
        paths[page->label()].append(undonePaths.takeLast());
        repaint();
        emit pathsChangedQuick(page->label(), paths[page->label()], shiftx, shifty, resolution);
    }
}

void PresentationSlide::clearLists()
{
    MediaSlide::clearLists();
    qDeleteAll(undonePaths);
    undonePaths.clear();
}

void PresentationSlide::endAnimation()
{
    stopAnimation();
    repaint();
    if (glitter != nullptr) {
        delete[] glitter;
        glitter = nullptr;
    }
    emit sendAdaptPage();
    updatePathCache();
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
    if (pointer_visible)
        hidePointer();
    else
        showPointer();
}

void PresentationSlide::showPointer()
{
    pointer_visible = true;
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
}

void PresentationSlide::hidePointer()
{
    pointer_visible = false;
    setCursor(Qt::BlankCursor);
    if (tool.tool != Pointer)
        setMouseTracking(false);
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
        drawPaths(painter, label, true);
    }
    {
        picfinal = QPixmap(size());
        picfinal.fill(QColor(0,0,0,0));
        QPainter painter;
        painter.begin(&picfinal);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPixmap(shiftx, shifty, pixmap);
        drawPaths(painter, page->label(), true);
    }
}

void PresentationSlide::animate(int const oldPageIndex) {
    if (oldPageIndex != pageIndex) {
        end_cache = -1;
         if (!pixpaths.isNull())
             pixpaths = QPixmap();
    }
    if (duration>-1e-6 && duration < .05) {
        transition_duration = 0;
        update();
        return;
    }
    if (transition_duration < 0 || oldPageIndex == pageIndex) {
        remainTimer.start(0);
        return;
    }
    timer.stop();
    remainTimer.stop();
    if (pixmap.isNull()) {
        transition_duration = 0;
        remainTimer.start(0);
        return;
    }
    picwidth = quint16(pixmap.width());
    picheight = quint16(pixmap.height());
    Poppler::PageTransition const* transition = page->transition();
    if (transition == nullptr) {
        transition_duration = 0;
        remainTimer.start(0);
        return;
    }
    if (oldPageIndex >= 0 && oldPageIndex > pageIndex)
        transition = doc->getPage(oldPageIndex)->transition();
    transition_duration = static_cast<qint32>(1000*transition->durationReal());
    if (transition_duration < 5) {
        remainTimer.start(0);
        return;
    }
    updateImages(oldPageIndex);
    remainTimer.setInterval(transition_duration-2);
    switch (transition->type()) {
    case Poppler::PageTransition::Replace:
        {
        transition_duration = 0;
        remainTimer.start(0);
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
        initGlitter();
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
        qint16 angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
        if ((oldPageIndex < pageIndex) ^ (transition->direction() == Poppler::PageTransition::Outward)) {
            if (transition->scale() < .999999) {
                transition_duration = static_cast<qint32>(transition_duration/(1.-transition->scale()));
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
        qint16 angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
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
        qint16 angle = (360 + transition->angle()) % 360;
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
        qint16 angle = (360 + 180*(oldPageIndex > pageIndex) + transition->angle()) % 360;
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
    emit requestUpdateNotes(pageIndex, false);
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

void PresentationSlide::paintGlitter(QPainter& painter)
{
    if (glitter == nullptr)
        return;
    qint32 const n = width()*height()/glitterpixel, w = width()/glitterpixel;
    qint16 const steps = qint16((nglitter*(transition_duration - remainTimer.remainingTime()))/transition_duration);
    painter.drawPixmap(0, 0, picinit);
    for (qint16 j=0; j<steps; j++) {
        for (qint32 i=glitter[j]%nglitter; i<n; i+=nglitter) {
            painter.drawPixmap(glitterpixel*(i%w), glitterpixel*(i/w), picfinal, glitterpixel*(i%w), glitterpixel*(i/w), glitterpixel, glitterpixel);
        }
    }
}

void PresentationSlide::initGlitter()
{
    unsigned int seed = this->seed + static_cast<unsigned int>(pageIndex+transition_duration);
    if (glitter != nullptr)
        delete[] glitter;
    glitter = new quint16[nglitter];
    for (quint16 i=0; i<nglitter; i++)
        glitter[i] = i;
    std::shuffle(glitter, glitter+nglitter, std::default_random_engine(seed));
}

void PresentationSlide::setGlitterSteps(quint16 const number)
{
    if (glitter != nullptr) {
        delete[] glitter;
        glitter = nullptr;
    }
    nglitter = number;
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
