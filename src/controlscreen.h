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
#include <QFileDialog>
#include <QLabel>
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
    void renderPage(int const pageNumber, bool const full = true);
    void setPagePart(PagePart const pagePart);
    void setColor(QColor const bgColor = Qt::gray, QColor const textColor = Qt::black);
    void setPresentationColor(QColor const color = Qt::black);
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    void setEmbedFileList(const QStringList &files);
    void setPid2WidConverter(QString const &program);
#endif
    void setUrlSplitCharacter(QString const &splitCharacter);
    void setScrollDelta(int const scrollDelta);
    void setForceTouchpad();
    void setCacheNumber(int const number);
    void setCacheSize(qint64 const size);
    void setTocLevel(quint8 const level);
    void setOverviewColumns(quint8 const columns) {overviewColumns=columns;}
    void setRenderer(QStringList command);
    void setKeyMap(QMap<quint32, QList<KeyAction>>* keymap);
    void unsetKeyMapItem(quint32 const key) {keymap->remove(key);}
    void setKeyMapItem(quint32 const key, KeyAction const action);
    void setTimerMap(QMap<int, QTime>& timeMap);
    void setToolForKey(quint32 const key, ColoredDrawTool tool) {tools[key] = tool;}
    void setMagnification(qreal const mag);
    void setAutostartDelay(double const timeout);
    void showDrawSlide();
    void hideDrawSlide();
    void loadDrawings(QString const& filename) {presentationScreen->slide->loadDrawings(filename);}
    ToolSelector* getToolSelector();
    PresentationSlide* getPresentationSlide() {return presentationScreen->slide;}
    // TODO: restructure cache management, return all images separately?

protected:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);

private:
    void recalcLayout(int const pageNumber);
    void reloadFiles();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    void startAllEmbeddedApplications();
#endif
    Ui::ControlScreen *ui;
    PresentationScreen* presentationScreen;
    PdfDoc* presentation;
    PdfDoc* notes;
    QTimer* cacheTimer = new QTimer(this);
    CacheUpdateThread* cacheThread = new CacheUpdateThread(this);
    TocBox* tocBox = nullptr;
    OverviewBox* overviewBox = nullptr;
    quint8 overviewColumns = 5;
    int numberOfPages;
    int currentPageNumber = 0;
    PagePart pagePart = FullPage;
    bool forceIsTouchpad = false;
    int scrollDelta = 200;
    int scrollState = 0;
    int maxCacheNumber = 10;
    qint64 maxCacheSize = 104857600;
    int first_delete = 0;
    int last_delete;
    int first_cached = 0;
    int last_cached = -1;
    qint64 cacheSize = 0;
    int cacheNumber = 0;
    QSize oldSize = QSize();
    DrawSlide* drawSlide = nullptr;

    // keymap maps (key code + modifiers) to a list of KeyActions.
    QMap<quint32, QList<KeyAction>>* keymap = new QMap<quint32, QList<KeyAction>>();
    QMap<quint32, ColoredDrawTool> tools;

private slots:
    void updateCacheStep();

signals:
    void sendNewPageNumber(int const pageNumber, bool const setDuration);
    void sendTimerString(QString const timerString);
    void sendTimerColors(QList<qint32> times, QList<QColor> colors);
    void sendCloseSignal();
    void playMultimedia();
    void pauseMultimedia();
    void sendAnimationDelay(quint32 const delay_ms);

public slots:
    // TODO: Some of these functions are not used as slots. Tidy up!
    void handleKeyAction(KeyAction const action);
    void receiveCache(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index);
    void receiveDest(QString const& dest);
    void receivePreviousSlideEnd(); // go to last overlay of previous slide
    void receiveNextSlideStart(); // go to first overlay of next slide
    void receiveNewPageNumber(int const pageNumber);
    void receivePageShiftEdit(int const shift = 0);
    void adaptPage();
    void receiveTimerAlert();
    void resetTimerAlert();
    void resetFocus();
    void focusPageNumberEdit();
    void addMultimediaSliders(int const n);
    void interconnectMultimediaSliders(int const n);
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
