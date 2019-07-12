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

#ifndef TRANSITIONWIDGET_H
#define TRANSITIONWIDGET_H

#include <QObject>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <poppler-page-transition.h>
#include "drawslide.h"

class PresentationSlide : public DrawSlide
{
    Q_OBJECT

private:
    int elapsed = 0;  // in ms
    int transition_duration = 0; // in ms
    int dt = 25;      // in ms
    int n_blinds = 8;
    int picwidth;
    int picheight;
    QTimer timer;
    void (PresentationSlide::*paint)(QPainter&);

protected:
    QTimer* const timeoutTimer = new QTimer(this);
    int minimumAnimationDelay = 40; // minimum frame time in ms
    QPixmap picinit;
    double duration = -1.; // duration of the current page in s
    void paintEvent(QPaintEvent*) override;
    void animate() override;
    void endAnimation() override;
    void setDuration() override;
    void enableTransitions() {elapsed=0;}

public:
    PresentationSlide(QWidget* parent=nullptr);
    ~PresentationSlide() override;
    QPixmap const& getCurrentPixmap() const {return pixmap;}
    void disableTransitions();
    double getDuration() const {return duration;}
    void setFrameTime(int const time) {dt=time;}
    void setBlinds(int const blinds) {n_blinds=blinds;}
    void paintSplitHI(QPainter& painter);
    void paintSplitVI(QPainter& painter);
    void paintSplitHO(QPainter& painter);
    void paintSplitVO(QPainter& painter);
    void paintBlindsH(QPainter& painter);
    void paintBlindsV(QPainter& painter);
    void paintBoxO(QPainter& painter);
    void paintBoxI(QPainter& painter);
    void paintWipeUp(QPainter& painter);
    void paintWipeDown(QPainter& painter);
    void paintWipeLeft(QPainter& painter);
    void paintWipeRight(QPainter& painter);
    void paintDissolve(QPainter& painter);
    void paintGlitter(QPainter& painter);
    void paintFly(QPainter& painter);
    void paintPushUp(QPainter& painter);
    void paintPushDown(QPainter& painter);
    void paintPushLeft(QPainter& painter);
    void paintPushRight(QPainter& painter);
    void paintCoverUp(QPainter& painter);
    void paintCoverDown(QPainter& painter);
    void paintCoverLeft(QPainter& painter);
    void paintCoverRight(QPainter& painter);
    void paintUncoverUp(QPainter& painter);
    void paintUncoverDown(QPainter& painter);
    void paintUncoverLeft(QPainter& painter);
    void paintUncoverRight(QPainter& painter);
    void paintFade(QPainter& painter);

public slots:
    void togglePointerVisibility();
    void setAnimationDelay(int const delay_ms) {minimumAnimationDelay=delay_ms;}

signals:
    void timeoutSignal();
};

typedef void (PresentationSlide::*paint)();

#endif // TRANSITIONWIDGET_H
