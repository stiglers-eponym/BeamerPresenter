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
#include <QOpenGLWidget>
#include <QDebug>
#include <QTimer>
#include <poppler-page-transition.h>
#include "pagewidget.h"

class PresentationWidget : public PageWidget
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
    QPainter painter;
    void (PresentationWidget::*paint)();

protected:
    QTimer* const timeoutTimer = new QTimer(this);
    int minimumAnimationDelay = 40; // minimum frame time in ms
    QPixmap picinit;
    void paintGL() override;
    void animate() override;
    void endAnimation() override;
    void setDuration() override;

public:
    PresentationWidget(QWidget* parent=nullptr);
    ~PresentationWidget() override;
    void setFrameTime(int const time) {dt=time;}
    void setBlinds(int const blinds) {n_blinds=blinds;}
    void paintSplitHI();
    void paintSplitVI();
    void paintSplitHO();
    void paintSplitVO();
    void paintBlindsH();
    void paintBlindsV();
    void paintBoxO();
    void paintBoxI();
    void paintWipeUp();
    void paintWipeDown();
    void paintWipeLeft();
    void paintWipeRight();
    void paintDissolve();
    void paintGlitter();
    void paintFly();
    void paintPushUp();
    void paintPushDown();
    void paintPushLeft();
    void paintPushRight();
    void paintCover();
    void paintUncover();
    void paintFade();

public slots:
    void setAnimationDelay(int const delay_ms) override {minimumAnimationDelay=delay_ms;}
};

typedef void (PresentationWidget::*paint)();

#endif // TRANSITIONWIDGET_H
