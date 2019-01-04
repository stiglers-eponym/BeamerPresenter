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
#include <QTimer>
#include <QMouseEvent>
#include <QDesktopServices>
#include <poppler-qt5.h>
#include "videowidget.h"

class PageLabel : public QLabel
{
    Q_OBJECT

public:
    PageLabel(QWidget* parent);
    PageLabel(Poppler::Page* page, QWidget* parent);
    ~PageLabel();
    void renderPage(Poppler::Page* page);
    int pageNumber();
    double getDuration() const;
    void setPresentationStatus(bool const status);
    void setShowMultimedia(bool const showVideos);
    bool hasActiveMultimediaContent() const;

private:
    QList<Poppler::Link*> links;
    QList<QRect*> linkPositions;
    QList<VideoWidget*> videoWidgets;
    QList<QRect*> videoPositions;
    QList<QMediaPlayer*> soundPlayers;
    QList<QRect*> soundPositions;
    bool isPresentation = true;
    bool showMultimedia = true;
    double autostartDelay = 0.; // delay for starting multimedia content in s
    QTimer * timer = nullptr;
    int minimumAnimationDelay = 20; // minimum frame time in ms

protected:
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent( QMouseEvent * event );

private:
    Poppler::Page* page;
    double duration;
    bool pointer_visible = true;

public slots:
    void togglePointerVisibility();
    void setAutostartDelay(double const delay);
    void pauseAllMultimedia();
    void startAllMultimedia();
    void setAnimationDelay(int const delay_ms);

signals:
    void sendNewPageNumber(int const pageNumber);
    void timeoutSignal();
};

#endif // PAGE_H
