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
#include "tocbox.h"
#include "overviewbox.h"
#include "toolselector.h"

// Namespace for te user interface from controlscreen.ui. I don't really know why.
// This probably follows a strange Qt convention.
namespace Ui {
    class ControlScreen;
}

/// Window on speaker screen, which controls the whole application.
class ControlScreen : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor and destructor
    explicit ControlScreen(QString presentationPath, QString notesPath = "", QWidget* parent = nullptr);
    ~ControlScreen() override;

    // Render pages on the control screen
    void renderPage(int const pageNumber, bool const full = true);
    // Update cache
    void updateCache();

    // Functions setting different properties from options (only used from main.cpp)
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
    void setCacheAll(bool const cache_all) {cacheAll = cache_all;}
    void setTocLevel(quint8 const level);
    void setOverviewColumns(quint8 const columns) {if (overviewBox != nullptr) overviewBox->setColumns(columns);}
    void setRenderer(QStringList command);
    void setKeyMap(QMap<quint32, QList<KeyAction>>* keymap);
    void setKeyMapItem(quint32 const key, KeyAction const action);
    void setTimerMap(QMap<int, QTime>& timeMap);
    void setToolForKey(quint32 const key, ColoredDrawTool tool) {tools[key] = tool;}
    void setMagnification(qreal const mag);
    void setAutostartDelay(double const timeout);
    void setTimerColors(QList<qint32> times, QList<QColor> colors);
    void setTimerString(QString const timerString);
    /// Configure minimum frame time for animations created by showing slides in rapid succession.
    void setAnimationDelay(quint32 const delay_ms) {presentationScreen->slide->setAnimationDelay(delay_ms);}
    void setMinSidebarWidth(double const sideWidth) {maxNotesWidth = 1. - sideWidth;}

    // Load drawings from file (also used only from main.cpp)
    void loadDrawings(QString const& filename) {presentationScreen->slide->loadDrawings(filename);}

    // Show or hide different widgets on the notes area.
    // This activates different modes: drawing, TOC, and overview mode.
    void showDrawSlide();
    void hideDrawSlide();
    void showToc();
    void hideToc();
    void showOverview();
    void hideOverview();

    // Get different widgets
    ToolSelector* getToolSelector();
    PresentationSlide* getPresentationSlide() {return presentationScreen->slide;}
    MediaSlide* getNotesSlide();

protected:
    // Handle events
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Update layout (if window size is changed).
    void recalcLayout(int const pageNumber);
    // Reload pdf files.
    void reloadFiles();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    // Start embedded applications on all slides.
    void startAllEmbeddedApplications();
#endif

    // User interface (created from controlscreen.ui)
    Ui::ControlScreen* ui;
    // Presentation Screen (own window containing presentation slide)
    PresentationScreen* presentationScreen;

    // PDF documents
    PdfDoc* presentation;
    PdfDoc* notes;

    // Objects handling cache: timer, and thread
    QTimer* cacheTimer = new QTimer(this);
    //CacheUpdateThread* cacheThread = new CacheUpdateThread(this);
    bool cacheAll = true;

    void cachePage(int const page);

    // Widgets shown above notes: TOC, overview, and drawSlide
    /// Widget showing the table of contents on the control screen.
    TocBox* tocBox = nullptr;
    /// Widget showing an overview of thumbnail slides on the control screen.
    OverviewBox* overviewBox = nullptr;
    /// Current presentation slide shown on the control screen,
    /// which can be used for drawing and is synchronized with the slide shown on the presentation screen.
    DrawSlide* drawSlide = nullptr;

    // Configuration. These values are set in main.cpp and will not be changed afterwards.
    /// Pages in presentation pdf contain both presentation and notes.
    PagePart pagePart = FullPage;
    /// Treat scrolling from all input devices as touch pad scrolling events.
    bool forceIsTouchpad = false;
    /// Number of pixels on a touch pad corresponding to scrolling one slide.
    int scrollDelta = 200;
    /// Maximum number of slides in cache.
    int maxCacheNumber = 10;
    /// Maximum size of cache in bytes. Note that cache can get larger than this size in some situations.
    qint64 maxCacheSize = 104857600L;
    /// Cached preview slides for standard sidebar width.
    CacheMap* previewCache = nullptr;
    /// Cached preview slides for different sidebar width.
    CacheMap* previewCacheX = nullptr;

    /// Maximum relative width of the notes slide.
    /// This equals one minus minimum width of the side bar.
    double maxNotesWidth = 0.8;
    // Key bindings.
    /// Map of key codes (key code + modifiers) to lists of KeyActions.
    QMap<quint32, QList<KeyAction>>* keymap = new QMap<quint32, QList<KeyAction>>();
    /// Map of key codes (key code + modifiers) to drawing tools.
    QMap<quint32, ColoredDrawTool> tools;

    // Variables
    /// Index of current page
    int currentPageNumber = 0;
    /// Scroll state for scrolling with touch pads: Single scroll events sum up to a total scroll event.
    /// If abs(scrollState) exceedes a threshold, the page is changed.
    int scrollState = 0;
    /// Size of currently shown slide.
    QSize oldSize;
    /// Total number of pages
    int numberOfPages;
    /// Number of slides in cache.
    int cacheNumber = 0;

    // Variables used for cache management
    /// All pages < first_delete are not saved in cache.
    /// Usually this page will be deleted when cache size should be reduced.
    int first_delete = 0;
    /// All pages > last_delete are not saved in cache.
    /// In some case this page will be deleted when cache size should be reduced.
    int last_delete;
    /// Beginning of the connected set of cached pages.
    int first_cached = 0;
    /// End of the connected set of cached pages.
    int last_cached = -1;
    /// Memory used by cache in bytes.
    qint64 cacheSize = 0;

private slots:
    /// Select a page which should be rendered to cache and free cache space if necessary.
    void updateCacheStep();

public slots:
    // TODO: Some of these functions are not used as slots. Tidy up!
    /// Handle actions sent from key event or tool selector.
    bool handleKeyAction(KeyAction const action);
    /// Receive a TOC destination and go the the corresponding slide.
    void receiveDest(QString const& dest);
    /// Go to last overlay of previous slide.
    void receivePreviousSlideEnd();
    /// Go to first overlay of next slide.
    void receiveNextSlideStart();
    /// Go to page.
    void receiveNewPageNumber(int const pageNumber);
    /// Shift page in notes without changing page in presentation..
    void receivePageShiftEdit(int const shift = 0); // TODO: remove?
    /// Synchronize presentation page to notes page.
    void adaptPage();
    /// Receive an alert from timer.
    void receiveTimerAlert();
    /// Deactivate a timer alert.
    void resetTimerAlert();
    /// Activate the control screen (window) and show the same in notes as in presentation.
    void resetFocus();
    /// Focus on page number editor. The page number can then be edited by entering a number.
    void focusPageNumberEdit();
    /// Add sliders on control screen to control multimedia content.
    void addMultimediaSliders(int const n);
    /// Clear cache.
    void clearPresentationCache();
    /// Show notes. This hides other widgets which can be shown above notes (TOC, overview, draw slide).
    void showNotes();
    /// Change cache size.
    void updateCacheSize(qint64 const diff);

signals:
    /// Send a new page number with or without starting a timer for the new slide.
    void sendNewPageNumber(int const pageNumber, bool const setDuration);
    /// Close the presentation window.
    void sendCloseSignal();
};

#endif // CONTROLSCREEN_H
