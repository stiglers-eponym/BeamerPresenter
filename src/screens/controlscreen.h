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
#include <QApplication>
#include "../pdf/pdfdoc.h"
#include "../gui/timer.h"
#include "../gui/pagenumberedit.h"
#include "presentationscreen.h"
#include "../gui/tocbox.h"
#include "../gui/overviewbox.h"
#include "../gui/toolselector.h"
#include "ui_controlscreen.h"

// Namespace for te user interface from controlscreen.ui. I don't really know why.
// This probably follows a Qt convention.
namespace Ui {
    class ControlScreen;
}

/// Window on speaker screen, which controls the whole application.
class ControlScreen : public QMainWindow
{
    Q_OBJECT

public:
    /// Construct control screen.
    /// Create the GUI including PresentationScreen and connect the widgets.
    explicit ControlScreen(QString presentationPath, QString notesPath = "", PagePart const part = FullPage, qreal const pagePartThreshold = -1., QWidget* parent = nullptr);
    /// Destructor. Delete the whole GUI.
    ~ControlScreen() override;

    // Render pages on the control screen
    void renderPage(int const pageNumber, bool const full = true);
    // Update cache
    void updateCache();

    // Functions setting different properties from options (only used from main.cpp)
    /// Set background and text color for control screen.
    void setColor(QColor const bgColor = Qt::gray, QColor const textColor = Qt::black);
    /// Set background color for presentation screen.
    void setPresentationColor(QColor const color = Qt::black);
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    void setEmbedFileList(const QStringList &files);
    void setPid2WidConverter(QString const &program);
#endif
    void setUrlSplitCharacter(QString const &splitCharacter);

    // Configure scrolling for touchpads.
    /// Scrolling using a touchpad: set number of pixels interpreted as one scroll step.
    void setScrollDelta(int const scrollDelta);
    /// Treat all scroll events as touch pad scroll events.
    void setForceTouchpad();

    // Configure cache.
    /// Set maximum number of slides, which should be rendered to cache.
    /// A negative number is interpreted as infinity.
    void setCacheNumber(int const number);
    /// Set maximum memory used for cached pages (in bytes).
    /// A negative number is interpreted as infinity.
    void setCacheSize(qint64 const size);
    /// Set maximum level of sections / subsections shown in the table of contents.
    void setTocLevel(quint8 const level);
    void setOverviewColumns(quint8 const columns) {if (overviewBox != nullptr) overviewBox->setColumns(columns);}
    void setRenderer(QStringList const& command);
    /// Set (overwrite) key bindings.
    void setKeyMap(QMap<quint32, QList<KeyAction>>* keymap);
    /// Add (key, action) to key bindings.
    void setKeyMapItem(quint32 const key, KeyAction const action);
    /// Add switching to given draw tool to key bindings for given key.
    void setToolForKey(quint32 const key, FullDrawTool const& tool);
    /// Set delay for multimedia content.
    void setAutostartDelay(qreal const timeout);
    /// Configure minimum frame time for animations created by showing slides in rapid succession.
    void setAnimationDelay(quint32 const delay_ms) {presentationScreen->slide->setAnimationDelay(delay_ms);}
    /// Set minimum relative width of the sidebar on control screen.
    void setMinSidebarWidth(qreal const sideWidth) {maxNotesWidth = 1. - sideWidth;}
    /// Enable or disable logging of slide changes to stdout.
    void setLogSlideChanges(bool const log) {ui->label_timer->setLog(log);}
    /// GUI Timer object handling presentation time.
    Timer* getTimer() {return ui->label_timer;}

    /// Load drawings from file (used only from main.cpp)
    void loadXML(QString const& filename) {presentationScreen->slide->getPathOverlay()->loadXML(filename, notes);}

    // Show or hide different widgets on the notes area.
    // This activates different modes: drawing, TOC, and overview mode.
    /// Draw mode: show draw slide instead of notes. This only has an effect if a separate notes file was loaded.
    void showDrawSlide();
    /// Exit draw mode. This only has an effect if a separate notes file was loaded.
    void hideDrawSlide();
    /// Show table of contents on the control screen (above the notes label).
    void showToc();
    /// Show overview on the control screen (above the notes label).
    void showOverview();

    // Get different widgets
    ToolSelector* getToolSelector() {return ui->tool_selector;}
    PresentationSlide* getPresentationSlide() {return presentationScreen->slide;}
    MediaSlide* getNotesSlide() {return ui->notes_widget;}

protected:
    // Handle events
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    /// Update layout if window size is changed.
    /// Adapt control screen based on the aspect ratios of presentation and notes slides.
    void recalcLayout(int const pageNumber);
    /// Reload pdf files if they have been updated.
    void reloadFiles();
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    /// Start embedded applications on all slides.
    void startAllEmbeddedApplications();
#endif
    /// Tell all cache processes to stop and wait up to <time> ms until each process is stopped.
    void interruptCacheProcesses(unsigned long const time = 0);
    /// Free a page from cache. Should only be called from updateCacheStep.
    bool freeCachePage(const int page);

    /// User interface (created from controlscreen.ui)
    Ui::ControlScreen* ui;
    /// Presentation Screen (own window containing presentation slide)
    PresentationScreen* presentationScreen;

    // PDF documents
    /// PDF document of the presentation.
    PdfDoc* presentation;
    /// PDF document of notes for the speaker. If no notes are given: notes == presentation.
    PdfDoc* notes;

    /// Timer for regular calls to cachePage.
    QTimer* cacheTimer = new QTimer(this);

    /// Cache given page on all slide widgets which can handle cache.
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
    /// Cached draw slide.
    CacheMap* drawSlideCache = nullptr;

    /// Maximum relative width of the notes slide.
    /// This equals one minus minimum width of the side bar.
    qreal maxNotesWidth = 0.8;
    // Key bindings.
    /// Map of key codes (key code + modifiers) to lists of KeyActions.
    QMap<quint32, QList<KeyAction>>* keymap = new QMap<quint32, QList<KeyAction>>();
    /// Map of key codes (key code + modifiers) to drawing tools.
    QMap<quint32, FullDrawTool> tools;

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
    /// Number of currently running cache threads.
    int cacheThreadsRunning = 0;

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
    /// Return true if no further key actions should be handled.
    bool handleKeyAction(KeyAction const action);
    /// Receive a TOC destination and go the the corresponding slide.
    void receiveDest(QString const& dest);
    /// Go to last overlay of previous slide.
    void receivePreviousSlideEnd();
    /// Go to first overlay of next slide.
    void receiveNextSlideStart();
    /// Go to page.
    void receiveNewPageNumber(int const pageNumber);
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
    /// Resize presentation window: clear cache, update tool sizes.
    void presentationResized();
    /// Show notes. This hides other widgets which can be shown above notes (TOC, overview, draw slide).
    void showNotes();
    /// Change cache size.
    void updateCacheSize(qint64 const diff) {cacheSize += diff;}
    /// Check whether cache threads finished.
    void cacheThreadFinished();
    /// Send draw tool from tool selector to draw slide and presentation.
    void distributeTools(FullDrawTool const& tool);
    void distributeStylusTools(FullDrawTool const& tool);

signals:
    /// Send a new page number with or without starting a timer for the new slide.
    void sendNewPageNumber(int const pageNumber, bool const setDuration);
    /// Close the presentation window.
    void sendCloseSignal();
};

#endif // CONTROLSCREEN_H
