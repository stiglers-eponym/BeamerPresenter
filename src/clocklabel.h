/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef CLOCKLABEL_H
#define CLOCKLABEL_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QLabel>

class ClockLabel : public QLabel
{
    Q_OBJECT

public:
    ClockLabel(QWidget* parent = nullptr);
    ~ClockLabel();

private:
    QTimer * timer = nullptr;

private slots:
    void showTime();
};

#endif // CLOCKLABEL_H
