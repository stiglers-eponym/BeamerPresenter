/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef PAGE_H
#define PAGE_H

#include <iostream>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <poppler-qt5.h>

class PageLabel : public QLabel
{
    Q_OBJECT

public:
    PageLabel(QWidget* parent);
    PageLabel(Poppler::Page* page, QWidget* parent);
    ~PageLabel();
    void renderPage(Poppler::Page* page);
    int pageNumber();
    QList<Poppler::Link*> links;
    QList<QRect*> linkPositions;

protected:
    void mouseReleaseEvent(QMouseEvent * event);

private:
    Poppler::Page* page;

public slots:

signals:
    void sendNewPageNumber(int const pageNumber);
};

#endif // PAGE_H
