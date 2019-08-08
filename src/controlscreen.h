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

#ifndef CONTROLSCREEN_H
#define CONTROLSCREEN_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QFileInfo>
#include <QSlider>
#include "pdfdoc.h"
#include "timer.h"
#include "pagenumberedit.h"
#include "presentationscreen.h"
#include "cacheupdatethread.h"
#include "tocbox.h"
#include "overviewbox.h"
#include "toolselector.h"

namespace Ui {
    class ControlScreen;
}

class ControlScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlScreen(QString presentationPath, QString notesPath = "", QWidget* parent = nullptr);
    ~ControlScreen();
    void renderPage(int const pageNumber);
    void setPagePart(PagePart const pagePart);
    void setColor(QColor const bgColor = Qt::gray, QColor const textColor = Qt::black);
    void setPresentationColor(QColor const color = Qt::black);
    void setEmbedFileList(const QStringList &files);
    void setPid2WidConverter(QString const &program);
    void setUrlSplitCharacter(QString const &splitCharacter);
    void setScrollDelta(int const scrollDelta);
    void setForceTouchpad();
    void setCacheNumber(int const number);
    void setCacheSize(long int const size);
    void setTocLevel(int const level);
    void setOverviewColumns(int const columns) {overviewColumns=columns;}
    void setRenderer(QStringList command);
    void setCacheVideos(bool const cache) {presentationScreen->setCacheVideos(cache);}
    void setKeyMap(QMap<int, QList<int>>* keymap);
    void unsetKeyMapItem(const int key) {keymap->remove(key);}
    void setKeyMapItem(const int key, const int action);
    void setTimerMap(QMap<int, QTime>& timeMap);
    void disableSlideTransitions() {presentationScreen->slide->disableTransitions();}
    void setToolSize(DrawTool const tool, int const size) {presentationScreen->slide->setSize(tool, size);}
    void setTransitionBlinds(int const n) {presentationScreen->slide->setBlindsNumber(n);}
    void showDrawSlide();
    void hideDrawSlide();
    // TODO: restructure cache management, return all images separately?

protected:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);

private:
    void recalcLayout(int const pageNumber);
    void reloadFiles();
    void startAllEmbeddedApplications();
    Ui::ControlScreen *ui;
    PresentationScreen* presentationScreen;
    PdfDoc* presentation;
    PdfDoc* notes;
    QTimer* cacheTimer = new QTimer(this);
    CacheUpdateThread* cacheThread = new CacheUpdateThread(this);
    TocBox* tocBox = nullptr;
    OverviewBox* overviewBox = nullptr;
    int overviewColumns = 5;
    int numberOfPages;
    int currentPageNumber = 0;
    PagePart pagePart = FullPage;
    bool forceIsTouchpad = false;
    int scrollDelta = 200;
    int scrollState = 0;
    int maxCacheNumber = 10;
    long int maxCacheSize = 104857600;
    int first_delete = 0;
    int last_delete;
    int first_cached = 0;
    int last_cached = -1;
    long int cacheSize = 0;
    int cacheNumber = 0;
    QSize oldSize = QSize();
    DrawSlide* drawSlide = nullptr;

    // keymap maps (key code + modifiers) to a list of KeyActions.
    QMap<int, QList<int>>* keymap = new QMap<int, QList<int>>();

private slots:
    void updateCacheStep();

signals:
    void togglePointerVisibilitySignal();
    void sendNewPageNumber(int const pageNumber);
    void sendTimerString(QString const timerString);
    void sendTimerColors(QList<int> times, QList<QColor> colors);
    void sendAutostartDelay(double const timeout);
    void sendAutostartEmbeddedDelay(double const timeout);
    void sendCloseSignal();
    void playMultimedia();
    void pauseMultimedia();
    void sendAnimationDelay(int const delay_ms);

public slots:
    // TODO: Some of these functions are not used as slots. Tidy up!
    void receiveCache(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index);
    void receiveDest(QString const& dest);
    void receivePreviousSlideEnd();
    void receiveNextSlideStart();
    void receiveNewPageNumber(int const pageNumber);
    void receivePageShiftEdit(int const shift = 0);
    void receivePageShiftReturn(int const shift = 0);
    void receiveCloseSignal();
    void receiveTimerAlert();
    void resetTimerAlert();
    void resetFocus();
    void focusPageNumberEdit();
    void addMultimediaSliders(int const n);
    void updateCache();
    void clearPresentationCache();
    void showToc();
    void hideToc();
    void showNotes();
    void showOverview();
    void hideOverview();
    void toggleDrawMode();
};

#endif // CONTROLSCREEN_H
