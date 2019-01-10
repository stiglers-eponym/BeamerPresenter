/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef MEDIASLIDER_H
#define MEDIASLIDER_H

#include <QObject>
#include <QWidget>
#include <QSlider>
#include <QKeyEvent>
#include <iostream>

class MediaSlider : public QSlider
{
    Q_OBJECT

public:
    MediaSlider(QWidget * parent = nullptr);
    ~MediaSlider();
    void setMaximum(qint64 const max);
    void setValue(qint64 const value);

protected:
    void keyPressEvent(QKeyEvent * event);

public slots:
    void emitSliderMoved(int const value);

signals:
    void sliderMoved(qint64 const value);
    void sendKeyEvent(QKeyEvent * event);
    void sendEscapeEvent();
};

#endif // MEDIASLIDER_H
