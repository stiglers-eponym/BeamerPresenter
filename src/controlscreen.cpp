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

#include "controlscreen.h"
#include "ui_controlscreen.h"

// TODO: tidy up! Separate and reorganize cache management; reorganize signals, slots, events, ...

ControlScreen::ControlScreen(QString presentationPath, QString notesPath, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ControlScreen)
{
    { // Check if files are valid
        if (presentationPath.isEmpty()) {
            qCritical() << "No presentation file specified";
            close();
            deleteLater();
            exit(1);
        }
        QFileInfo checkPresentation(presentationPath);
        if (!checkPresentation.exists() || (!checkPresentation.isFile() && !checkPresentation.isSymLink()) ) {
            qCritical() << "Not a file: " << presentationPath;
            close();
            deleteLater();
            exit(1);
        }
        if (notesPath == presentationPath)
            notesPath = "";
        else if (!notesPath.isEmpty()) {
            QFileInfo checkNotes(notesPath);
            if (!checkNotes.exists() || (!checkNotes.isFile() && !checkNotes.isSymLink()) ) {
                qCritical() << "Ignoring invalid notes files: " << notesPath;
                notesPath = "";
            }
        }
    }
    // Create GUI
    ui->setupUi(this);

    // Load presentation pdf
    presentation = new PdfDoc(presentationPath);
    if (!presentation->loadDocument()) {
        qCritical() << "Could not open document: " << presentationPath;
        close();
        deleteLater();
        exit(1);
    }
    numberOfPages = presentation->getDoc()->numPages();

    // Some numbers for cache management
    maxCacheNumber = numberOfPages;
    last_delete = numberOfPages-1;

    // Set up presentation screen
    presentationScreen = new PresentationScreen(presentation);
    presentationScreen->setWindowTitle("BeamerPresenter: " + presentationPath);

    // Load notes pdf
    if (!notesPath.isEmpty()) {
        notes = new PdfDoc(notesPath);
        if (notes->loadDocument())
            setWindowTitle("BeamerPresenter: " + notesPath);
        else {
            qCritical() << "File could not be opened as PDF: " << notesPath;
            notesPath = "";
        }
    }
    if (notesPath.isEmpty()) {
        notes = presentation;
        setWindowTitle("BeamerPresenter: " + presentationPath);
        ui->notes_widget->clearAll();
        ui->notes_widget = new DrawSlide(this);
        ui->notes_widget->setFocusPolicy(Qt::ClickFocus);
        drawSlide = static_cast<DrawSlide*>(ui->notes_widget);
        connect(ui->tool_selector, &ToolSelector::sendNewTool, drawSlide, QOverload<const ColoredDrawTool>::of(&DrawSlide::setTool));
        connect(drawSlide, &DrawSlide::pathsChangedQuick, presentationScreen->slide, &DrawSlide::setPathsQuick);
        connect(presentationScreen->slide, &DrawSlide::pathsChangedQuick, drawSlide, &DrawSlide::setPathsQuick);
        connect(drawSlide, &DrawSlide::pathsChanged, presentationScreen->slide, &DrawSlide::setPaths);
        connect(presentationScreen->slide, &DrawSlide::pathsChanged, drawSlide, &DrawSlide::setPaths);
        connect(drawSlide, &DrawSlide::pointerPositionChanged, presentationScreen->slide, &DrawSlide::setPointerPosition);
        connect(presentationScreen->slide, &DrawSlide::pointerPositionChanged, drawSlide, &DrawSlide::setPointerPosition);
        connect(drawSlide, &DrawSlide::sendRelax, presentationScreen->slide, &DrawSlide::relax);
        connect(presentationScreen->slide, &DrawSlide::sendRelax, drawSlide, &DrawSlide::relax);
        connect(drawSlide, &DrawSlide::sendUpdateEnlargedPage, presentationScreen->slide, &DrawSlide::updateEnlargedPage);
        connect(presentationScreen->slide, &DrawSlide::sendUpdateEnlargedPage, drawSlide, &DrawSlide::updateEnlargedPage);
        connect(drawSlide, &DrawSlide::sendUpdatePathCache, presentationScreen->slide, &DrawSlide::updatePathCache);
        connect(presentationScreen->slide, &DrawSlide::sendUpdatePathCache, drawSlide, &DrawSlide::updatePathCache);
        connect(drawSlide, &MediaSlide::sendPlayVideo,  presentationScreen->slide, &MediaSlide::playVideo);
        connect(drawSlide, &MediaSlide::sendPauseVideo, presentationScreen->slide, &MediaSlide::pauseVideo);
        connect(presentationScreen->slide, &MediaSlide::sendPlayVideo,  drawSlide, &MediaSlide::playVideo);
        connect(presentationScreen->slide, &MediaSlide::sendPauseVideo, drawSlide, &MediaSlide::pauseVideo);
        drawSlide->setMuted(true);
    }
    ui->notes_widget->setDoc(notes);
    ui->current_slide->setDoc(presentation);
    ui->next_slide->setDoc(presentation);

    // tocBox will be used to show the table of contents on the control screen
    tocBox = new TocBox(this);
    tocBox->setGeometry(ui->notes_widget->geometry());
    tocBox->hide();

    // overviewBox will be used to show an overview of all slides on the control screen
    overviewBox = new OverviewBox(this);
    overviewBox->setGeometry(ui->notes_widget->geometry());
    overviewBox->hide();

    // Set up the widgets
    ui->text_number_slides->setText(QString::number(numberOfPages));
    ui->text_current_slide->setNumberOfPages(numberOfPages);
    ui->next_slide->setUseCache(0);
    ui->notes_widget->setFocus();

    // Tool selector
    connect(ui->tool_selector, &ToolSelector::sendNewTool, presentationScreen->slide, QOverload<const ColoredDrawTool>::of(&DrawSlide::setTool));
    connect(ui->tool_selector, &ToolSelector::sendAction, this, &ControlScreen::handleKeyAction);


    // Page requests from the labels:
    // These are emitted if links are clicked.
    // These events are send to ControlScreen and PresentationScreen
    connect(ui->notes_widget,  &PreviewSlide::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->current_slide, &PreviewSlide::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->next_slide,    &PreviewSlide::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->notes_widget,  &PreviewSlide::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPage);
    connect(ui->current_slide, &PreviewSlide::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPage);
    connect(ui->next_slide,    &PreviewSlide::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPage);

    connect(ui->notes_widget,  &PreviewSlide::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(ui->current_slide, &PreviewSlide::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(ui->next_slide,    &PreviewSlide::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(presentationScreen->slide, &PreviewSlide::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);

    connect(ui->notes_widget,  &PreviewSlide::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->current_slide, &PreviewSlide::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->next_slide,    &PreviewSlide::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(presentationScreen->slide, &PreviewSlide::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->notes_widget,  &PreviewSlide::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(ui->current_slide, &PreviewSlide::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(ui->next_slide,    &PreviewSlide::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(presentationScreen->slide, &PreviewSlide::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);

    // Navigation signals emitted by PresentationScreen:
    connect(presentationScreen->slide, &PresentationSlide::sendAdaptPage, this, &ControlScreen::adaptPage);
    connect(presentationScreen, &PresentationScreen::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(presentationScreen->slide, &PresentationSlide::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);

    // Other signals emitted by PresentationScreen
    connect(presentationScreen, &PresentationScreen::sendKeyEvent,    this, &ControlScreen::keyPressEvent);
    connect(presentationScreen, &PresentationScreen::sendCloseSignal, this, &ControlScreen::close);
    connect(presentationScreen->slide, &MediaSlide::requestMultimediaSliders, this, &ControlScreen::addMultimediaSliders);
    if (drawSlide!=nullptr)
        connect(drawSlide, &MediaSlide::requestMultimediaSliders, this, &ControlScreen::interconnectMultimediaSliders);
    connect(presentationScreen, &PresentationScreen::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);

    // Signals sent back to PresentationScreen
    connect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::renderPage);
    connect(this, &ControlScreen::sendCloseSignal,   presentationScreen, &PresentationScreen::close);
    connect(ui->notes_widget, &PreviewSlide::sendCloseSignal, presentationScreen, &PresentationScreen::close);
    connect(presentationScreen->slide, &PreviewSlide::sendCloseSignal, presentationScreen, &PresentationScreen::close);
    connect(ui->notes_widget, &PreviewSlide::sendCloseSignal, this, &ControlScreen::close);
    connect(presentationScreen->slide, &PreviewSlide::sendCloseSignal, this, &ControlScreen::close);
    connect(presentationScreen->slide, &PresentationSlide::requestUpdateNotes, this, &ControlScreen::renderPage);

    ui->label_timer->setTimerWidget(ui->edit_timer);
    // Signals emitted by the timer
    connect(ui->label_timer, &Timer::sendAlert,   this, &ControlScreen::receiveTimerAlert);
    connect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    connect(ui->label_timer, &Timer::sendEscape,  this, &ControlScreen::resetFocus);
    // Signals sent back to the timer
    connect(this, &ControlScreen::sendTimerString, ui->label_timer, &Timer::receiveTimerString);
    connect(this, &ControlScreen::sendTimerColors, ui->label_timer, &Timer::receiveColors);
    connect(presentationScreen, &PresentationScreen::pageChanged, ui->label_timer, &Timer::setPage);

    // Signals sent to the page labels
    //connect(this, &ControlScreen::playMultimedia,     ui->notes_widget, &MediaSlide::startAllMultimedia);
    connect(this, &ControlScreen::playMultimedia,     presentationScreen->slide, &MediaSlide::startAllMultimedia);
    connect(this, &ControlScreen::pauseMultimedia,    ui->notes_widget, &MediaSlide::pauseAllMultimedia);
    connect(this, &ControlScreen::pauseMultimedia,    presentationScreen->slide, &MediaSlide::pauseAllMultimedia);
    connect(this, &ControlScreen::sendAnimationDelay, presentationScreen->slide, &PresentationSlide::setAnimationDelay);

    // Signals emitted by the page number editor
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPage);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit,   this, &ControlScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit,    this, &ControlScreen::receivePageShiftEdit);
    connect(ui->text_current_slide, &PageNumberEdit::sendNextSlideStart,   this, &ControlScreen::receiveNextSlideStart);
    connect(ui->text_current_slide, &PageNumberEdit::sendPreviousSlideEnd, this, &ControlScreen::receivePreviousSlideEnd);
    connect(ui->text_current_slide, &PageNumberEdit::sendEscape,           this, &ControlScreen::resetFocus);

    // Cache handling
    connect(cacheTimer, &QTimer::timeout, this, &ControlScreen::updateCacheStep);
    cacheThread->setSlideWidgets(presentationScreen->slide, ui->notes_widget, ui->current_slide);
    connect(cacheThread, &CacheUpdateThread::resultsReady, this, &ControlScreen::receiveCache);
    connect(presentationScreen, &PresentationScreen::clearPresentationCacheRequest, this, &ControlScreen::clearPresentationCache);

    // TOC
    connect(tocBox, &TocBox::sendDest, this, &ControlScreen::receiveDest);

    // Overview box
    connect(overviewBox, &OverviewBox::sendPageNumber, presentationScreen, [&](int const pageNumber){presentationScreen->renderPage(pageNumber, false);});
    connect(overviewBox, &OverviewBox::sendPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(overviewBox, &OverviewBox::sendReturn, this, &ControlScreen::showNotes);
}

ControlScreen::~ControlScreen()
{
    delete tocBox;
    delete overviewBox;
    cacheThread->requestInterruption();
    cacheTimer->stop();
    cacheTimer->disconnect();
    delete cacheTimer;
    if (cacheThread->isRunning()) {
        cacheThread->quit();
        cacheThread->wait();
    }
    delete cacheThread;
    if (drawSlide != nullptr && drawSlide != ui->notes_widget)
        drawSlide->disconnect();
    ui->notes_widget->disconnect();
    ui->current_slide->disconnect();
    ui->next_slide->disconnect();
    ui->label_timer->disconnect();
    ui->text_current_slide->disconnect();
    presentationScreen->slide->disconnect();
    presentationScreen->disconnect();
    if (notes != presentation)
        delete notes;
    delete keymap;
    if (drawSlide != ui->notes_widget)
        delete drawSlide;
    delete presentation;
    delete presentationScreen;
    disconnect();
    delete ui;
}

void ControlScreen::setPagePart(PagePart const pagePart)
{
    // This is used for documents created with LaTeX beamer with \setbeameroption{show notes on second screen=[left or right]}
    this->pagePart = pagePart;
    switch (pagePart) {
        case FullPage:
            ui->notes_widget->setPagePart(FullPage);
            break;
        case LeftHalf:
            cacheThread->setPagePart(LeftHalf);
            ui->notes_widget->setPagePart(RightHalf);
            break;
        case RightHalf:
            cacheThread->setPagePart(RightHalf);
            ui->notes_widget->setPagePart(LeftHalf);
            break;
    }
    ui->current_slide->setPagePart(pagePart);
    ui->next_slide->setPagePart(pagePart);
    presentationScreen->slide->setPagePart(pagePart);
}

void ControlScreen::recalcLayout(const int pageNumber)
{
    // Calculate a good size for the notes side bar
    double screenRatio = double(height()) / width();
    if (screenRatio > 1)
        screenRatio = 1.;
    QSize notesSize = notes->getPageSize(pageNumber);
    if (drawSlide != nullptr && drawSlide != ui->notes_widget)
        notesSize = presentation->getPageSize(pageNumber);
    double notesSizeRatio = double(notesSize.height()) / notesSize.width();
    // Adjustment if the pdf includes slides and notes
    if (pagePart != FullPage)
        notesSizeRatio *= 2;
    // relative width of the notes slide on the control screen:
    double relativeNotesWidth = notesSizeRatio / screenRatio;
    if (relativeNotesWidth > 0.75)
        relativeNotesWidth = 0.75;
    // width of the sidebar:
    int sideWidth = int((1-relativeNotesWidth)*width());

    // Set layout
    ui->notes_widget->setGeometry(0, 0, width()-sideWidth, height());
    ui->current_slide->setMaximumWidth(sideWidth);
    ui->next_slide->setMaximumWidth(sideWidth);
    ui->gridLayout->setColumnStretch(0, width()-sideWidth);
    ui->gridLayout->setColumnStretch(1, sideWidth);
    tocBox->setGeometry(int(0.1*(width()-sideWidth)), 0, int(0.8*(width()-sideWidth)), height());
    overviewBox->setGeometry(0, 0, width()-sideWidth, height());
    ui->tool_selector->setMaximumWidth(sideWidth);
    if (drawSlide != nullptr) {
        double scale = drawSlide->getResolution() / presentationScreen->slide->getResolution();
        if (scale < 0.)
            scale = 1.;
        drawSlide->setSize(Pen, static_cast<quint16>(scale*presentationScreen->slide->getSize(Pen)+0.5));
        drawSlide->setSize(Pointer, static_cast<quint16>(scale*presentationScreen->slide->getSize(Pointer)+0.5));
        drawSlide->setSize(Highlighter, static_cast<quint16>(scale*presentationScreen->slide->getSize(Highlighter)+0.5));
        drawSlide->setSize(Torch, static_cast<quint16>(scale*presentationScreen->slide->getSize(Torch)));
        drawSlide->setSize(Magnifier, static_cast<quint16>(scale*presentationScreen->slide->getSize(Magnifier)));
        if (drawSlide != ui->notes_widget) {
            drawSlide->setGeometry(ui->notes_widget->rect());
            drawSlide->setScaledPixmap(presentationScreen->slide->getCurrentPixmap());
        }
    }
    updateGeometry();

    // Adjust font sizes
    if (sideWidth < 300) {
        QFont font = ui->label_timer->font();
        font.setPixelSize(sideWidth/6);
        ui->label_timer->setFont(font);
        font.setPixelSize(sideWidth/10);
        ui->edit_timer->setFont(font);
        font.setPixelSize(sideWidth/6);
        ui->label_clock->setFont(font);
    }
    else {
        QFont font = ui->label_timer->font();
        font.setPixelSize(50);
        ui->label_timer->setFont(font);
        font.setPixelSize(30);
        ui->edit_timer->setFont(font);
        font.setPixelSize(50);
        ui->label_clock->setFont(font);
    }
}

void ControlScreen::focusPageNumberEdit()
{
    this->activateWindow();
    ui->text_current_slide->setFocus();
}

void ControlScreen::addMultimediaSliders(int const n)
{
    // Add n sliders at the bottom of the sidebar
    QList<QSlider*> sliderList = QList<QSlider*>();
    for (int i=0; i<n; i++) {
        QSlider* slider = new QSlider(Qt::Horizontal, this);
        ui->overviewLayout->addWidget(slider);
        sliderList.append(slider);
    }
    // Send the sliders to the presentation label, where they will be connected to multimedia objects.
    // The presentation label takes ownership of the sliders and will delete them when going to the next slide.
    presentationScreen->slide->setMultimediaSliders(sliderList);
}

void ControlScreen::interconnectMultimediaSliders(int const n)
{
    if (drawSlide == nullptr || n!=presentationScreen->slide->getSliderNumber())
        return;
    drawSlide->connectVideoSliders(presentationScreen->slide->getVideoSliders());
}

void ControlScreen::resetFocus()
{
    // Make sure that the control screen shows the same page as the presentation screen and set focus to the notes label.
    if (currentPageNumber != presentationScreen->getPageNumber())
        renderPage(presentationScreen->getPageNumber());
    ui->notes_widget->setFocus();
}

void ControlScreen::receiveTimerAlert()
{
    QPalette palette = QPalette();
    palette.setColor(QPalette::Background, Qt::red);
    palette.setColor(QPalette::Base, Qt::red);
    ui->edit_timer->setPalette(palette);
}

void ControlScreen::resetTimerAlert()
{
    QPalette palette = QPalette();
    palette.setColor(QPalette::Background, Qt::darkGray);
    palette.setColor(QPalette::Base, Qt::white);
    ui->edit_timer->setPalette(palette);
}

void ControlScreen::renderPage(int const pageNumber, bool const full)
{
    // Update all page labels on the control screen to show the given page number.
    // This uses cached pages if such are available.

    // Negative page numbers are interpreted as signal for going to the last page.
    if (pageNumber < 0 || pageNumber >= numberOfPages)
        currentPageNumber = numberOfPages - 1;
    else
        currentPageNumber = pageNumber;

    if (!isVisible()) {
        ui->text_current_slide->setText(QString::number(currentPageNumber+1));
        if (full) {
            // Some extras which may take some time
            if (presentationScreen->slide->getTool().tool == Magnifier)
                presentationScreen->slide->updateEnlargedPage();
            presentationScreen->slide->updateCacheVideos(presentationScreen->pageIndex+1);
        }
        return;
    }

    // Recalculate layout if the window size has changed.
    if (full && size() != oldSize) {
        recalcLayout(currentPageNumber);
        oldSize = size();
    }

    if (drawSlide == nullptr) {
        // Update notes
        // The note document could have fewer pages than the presentation document:
        if (currentPageNumber < notes->getDoc()->numPages())
            ui->notes_widget->renderPage(currentPageNumber, false);
        else
            qWarning() << "Reached the end of the note file.";

        // Update current and next slide previews
        // If we have not reached the last page (there exists a next page):
        if (currentPageNumber + 1 < presentation->getDoc()->numPages()) {
            ui->current_slide->renderPage(currentPageNumber);
            if (maxCacheSize!=0 && maxCacheNumber!=0) {
                // Render the next slide to the current slide label's cache
                ui->current_slide->updateCache(currentPageNumber+1);
                // Now show the cached page on the next slide label
                QPixmap const pixmap = ui->current_slide->getCache(currentPageNumber+1);
                ui->next_slide->renderPage(currentPageNumber+1, &pixmap);
            }
            else if (full) // No cache: this is inefficient.
                ui->next_slide->renderPage(currentPageNumber+1);
        }
        else { // If we have reached the last page (there is no next page)
            // Get page and pixmap for current page.
            QPixmap pixmap = ui->current_slide->getCache(currentPageNumber);
            // Show the page on the current and next slide label.
            if (pixmap.isNull()) {
                // The page was not cached. Render it on current slide label.
                ui->current_slide->renderPage(currentPageNumber);
                pixmap = ui->current_slide->getCache(currentPageNumber);
            }
            else
                ui->current_slide->renderPage(currentPageNumber, &pixmap);
            ui->next_slide->renderPage(currentPageNumber, &pixmap);
        }
    }
    else {
        // It is possible that presentationScreen->slide contains drawings which have not been copied to drawSlide yet.
        QString label = presentation->getLabel(currentPageNumber);
        if (drawSlide->getPage() != nullptr && !drawSlide->getPaths().contains(label)) {
            qint16 const sx=presentationScreen->slide->getXshift(), sy=presentationScreen->slide->getYshift();
            double res = presentationScreen->slide->getResolution();
            drawSlide->setPaths(label, presentationScreen->slide->getPaths()[label], sx, sy, res);
        }
        // Update current slide
        drawSlide->renderPage(currentPageNumber, false);

        // Update next slide previews
        // If we have not reached the last page (there exists a next page):
        if (currentPageNumber + 2 < presentation->getDoc()->numPages()) {
            ui->current_slide->renderPage(currentPageNumber+1);
            if (maxCacheSize!=0 && maxCacheNumber!=0) {
                // Render the next slide to the current slide label's cache
                ui->current_slide->updateCache(currentPageNumber+2);
                // Now show the cached page on the next slide label
                QPixmap const pixmap = ui->current_slide->getCache(currentPageNumber+2);
                ui->next_slide->renderPage(currentPageNumber+2, &pixmap);
            }
            else // No cache: this is inefficient.
                ui->next_slide->renderPage(currentPageNumber+2);
        }
        else if (currentPageNumber + 2 == presentation->getDoc()->numPages()) {
            // Show the last two slides (current and next slide)
            ui->current_slide->renderPage(currentPageNumber);
            if (maxCacheSize!=0 && maxCacheNumber!=0) {
                // Render the next slide to the current slide label's cache
                ui->current_slide->updateCache(currentPageNumber+1);
                // Now show the cached page on the next slide label
                QPixmap const pixmap = ui->current_slide->getCache(currentPageNumber+1);
                ui->next_slide->renderPage(currentPageNumber+1, &pixmap);
            }
            else // No cache: this is inefficient.
                ui->next_slide->renderPage(currentPageNumber+2);
        }
        else { // If we have reached the last page (there is no next page)
            // Show the last slide twice
            // Get page and pixmap for current page.
            QPixmap pixmap = ui->current_slide->getCache(currentPageNumber);
            // Show the page on the current and next slide label.
            if (pixmap.isNull()) {
                // The page was not cached. Render it on current slide label.
                ui->current_slide->renderPage(currentPageNumber);
                pixmap = ui->current_slide->getCache(currentPageNumber);
            }
            else
                ui->current_slide->renderPage(currentPageNumber, &pixmap);
            ui->next_slide->renderPage(currentPageNumber, &pixmap);
        }
    }
    // Update the page number
    ui->text_current_slide->setText(QString::number(currentPageNumber+1));
    if (full) {
        // Some extras which may take some time
        if (presentationScreen->slide->getTool().tool == Magnifier) {
            ui->current_slide->repaint();
            ui->next_slide->repaint();
            presentationScreen->slide->updateEnlargedPage();
            if (drawSlide != nullptr)
                drawSlide->updateEnlargedPage();
        }
        presentationScreen->slide->updateCacheVideos(presentationScreen->pageIndex+1);
    }
}

void ControlScreen::updateCache()
{
    // (Re)start updating cache.
    // This (re)initializes the variables for cache management and starts the cacheTimer, which manages cache updates in a separate thread.

    if (maxCacheSize == 0 || maxCacheNumber == 0)
        // cache is disabled
        return;

    // Stop running cache updates
    cacheTimer->stop();
    // Number of currently cached slides
    cacheNumber = presentationScreen->slide->getCacheNumber();
    if (cacheNumber == numberOfPages)
        // All slides are cached
        return;
    // Size of currently cached slides in memory (set to -infinity if it should be ignored)
    if (maxCacheSize > 0)
        cacheSize = presentationScreen->slide->getCacheSize()
             + ui->notes_widget->getCacheSize()
             + ui->current_slide->getCacheSize();
    else
        // This is approximately -infinity and means that the cache size is unlimited:
        cacheSize = -8589934591L; // -8GiB

    // There should be a simply connected region of cached pages between first_cached and last_cached.
    if (first_cached > currentPageNumber || last_cached < currentPageNumber) {
        // We are outside the simply connected cache region.
        // Reset cache numbers and start a new simply connected region, which is initially empty.
        cacheThread->requestInterruption();
        first_cached = currentPageNumber;
        last_cached = currentPageNumber-1;
        first_delete = 0;
        last_delete = numberOfPages-1;
        qDebug() << "Reset cache region" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
    }
    else {
        // We are in the simply connected cache region.
        // Make sure that deleting cached pages starts outside this region.
        // Cached pages will only be deleted if the memory or the number of cached pages is limited.
        last_delete = last_delete > currentPageNumber + cacheNumber ? last_delete : currentPageNumber + cacheNumber;
        last_delete = last_delete >= numberOfPages ? numberOfPages-1 : last_delete;
        first_delete = first_delete > currentPageNumber - cacheNumber/2 ? currentPageNumber - cacheNumber/2 : first_delete;
        first_delete = first_delete < 0 ? 0 : first_delete;
    }
    if (last_cached < numberOfPages-1 || first_cached > 0)
        // The simply connected region does not include all pages.
        // Start the update steps by starting the cacheTimer.
        // cacheTimer will call updateCacheStep().
        cacheTimer->start();
}

void ControlScreen::updateCacheStep()
{
    /*
    * Select a page for rendering to cache and tell the cacheThread to render that page.
    * Delete cached pages if necessary due to limited memory or a limited number of cached slides.
    * This function will notice when no more pages need to be rendered to cache and stop the cacheTimer.
    *
    * Outline of the cache management:
    *
    * 0. updateCache initializes some variables and starts cacheTimer.
    * 1. cacheTimer calls updateCacheStep in a loop whenever the main thread is not busy
    * 2. updateCacheStep deletes pages as long as the cache uses too much memory (or as long as too many slides are cached).
    * 3. updateCacheStep checks whether a new pages should be rendered to cache.
    *    TODO: This process of checking and finding pages should be more understandable and deterministic.
    *    - If no more cached pages are needed, it stops cacheTimer and returns.
    *    - If it finds a page, which should be rendered to cache:
    *        a. it stops the cacheTimer
    *        b. it hands the page to cacheThread
    *        c. it starts cacheThread
    * 4. cacheThread renders the page in a different thread.
    *    This is where the main work is done. The rendering does not affect the main thread.
    *    cacheThread can optionally use an external renderer.
    *    When the rendering is done, cacheThread sends the results to receiveCache.
    * 5. receiveCache saves the cached pages to the pagewidgets' cache.
    *    If all pages have been rendered to cache, it sends an info message.
    *    Otherwise it starts cacheTimer again.
    */


    // TODO: improve this, make it more deterministic, avoid caching pages which will directly be freed again
    if (last_cached > last_delete || first_cached < first_delete || first_cached > currentPageNumber || last_cached < currentPageNumber-1 || cacheThread->isRunning()) {
        cacheTimer->stop();
        qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete << cacheThread->isRunning();
        return;
    }
    // Free space if necessary
    long int delta;
    while (cacheSize > maxCacheSize || cacheNumber > maxCacheNumber) {
        // Start deleting later slides if less than 1/4 of the caches slides are previous slides
        if (last_delete > 4*currentPageNumber - 3*first_delete) {
            delta = presentationScreen->slide->clearCachePage(last_delete);
            cacheSize -= ui->notes_widget->clearCachePage(last_delete)
                    + ui->current_slide->clearCachePage(last_delete);
            if (delta != 0) {
                cacheNumber--;
                cacheSize -= delta;
                qDebug() << "Freed last page" << last_delete << ". Cache size" << cacheSize << "B";
            }
            last_delete--;
            last_cached = last_delete > last_cached ? last_cached : last_delete;
        }
        else {
            delta = presentationScreen->slide->clearCachePage(first_delete);
            cacheSize -= ui->notes_widget->clearCachePage(first_delete)
                    + ui->current_slide->clearCachePage(first_delete);
            if (delta != 0) {
                cacheNumber--;
                cacheSize -= delta;
                qDebug() << "Freed first page" << first_delete << ". Cache size" << cacheSize << "B";
            }
            first_delete++;
            first_cached = first_cached > first_delete ? first_cached : first_delete;
        }
        if (last_cached > last_delete || first_cached < first_delete) {
            cacheTimer->stop();
            qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
            return;
        }
    }
    if (last_cached+1 == numberOfPages) {
        if (first_cached > first_delete && (maxCacheNumber==numberOfPages || 2*maxCacheNumber > 3*cacheNumber) && 2*maxCacheSize > 3*cacheSize) {
            // cache first_cached-1
            if (!presentationScreen->slide->cacheContains(first_cached-1)) {
                cacheTimer->stop();
                cacheThread->setPage(first_cached-1);
                cacheThread->start();
            }
            else {
                first_cached--;
                cacheSize += ui->notes_widget->updateCache(first_cached)
                             + ui->current_slide->updateCache(first_cached);
            }
            return;
        }
        else {
            cacheTimer->stop();
            qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
            return;
        }
    }
    // Don't continue if it is likely that the next cached page would directly be deleted.
    else if ( (last_cached == last_delete || 3*(last_cached-currentPageNumber+1) >= 2*maxCacheNumber)
            && (cacheNumber >= maxCacheNumber || (maxCacheSize-cacheSize)*cacheNumber < 2*cacheSize) ) {
        cacheTimer->stop();
        qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
        return;
    }
    else {
        // Cache the page last_cached+1
        if (!presentationScreen->slide->cacheContains(last_cached+1)) {
            cacheTimer->stop();
            cacheThread->setPage(last_cached+1);
            cacheThread->start();
        }
        else {
            // If render only labels for the control screen.
            // This might be inefficient if the control screen has been resized.
            // But usually the cached pages for presentation screen and control screen should not differ.
            last_cached++;
            cacheSize += ui->notes_widget->updateCache(last_cached)
                         + ui->current_slide->updateCache(last_cached);
        }
    }
}

void ControlScreen::receiveCache(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index)
{
    if (presentationScreen->slide->cacheContains(index)) {
        qDebug() << "Page is already cached:" << index << ". Cache size" << cacheSize << "B";
        presentationScreen->slide->updateCache(pres, index);
        ui->notes_widget->updateCache(note, index);
        ui->current_slide->updateCache(small, index);
    }
    else {
        cacheSize += presentationScreen->slide->updateCache(pres, index)
                     + ui->notes_widget->updateCache(note, index)
                     + ui->current_slide->updateCache(small, index);
        cacheNumber++;
        qDebug() << "Cached page" << index << "; Cache size" << cacheSize << "B";
        if (cacheNumber==numberOfPages) {
            qInfo() << "All slides are rendered to cache. Cache size:" << cacheSize << "B";
            first_cached = 0;
            last_cached = numberOfPages-1;
            return;
        }
    }
    if (index == last_cached+1)
        last_cached++;
    else if (index == first_cached-1)
        first_cached--;
    if (cacheThread->wait(100))
        cacheTimer->start();
}

void ControlScreen::setCacheNumber(int const number)
{
    // Set maximum number of slides, which should be rendered to cache.
    // A negative number is interpreted as infinity.
    if (number < 0)
        maxCacheNumber = numberOfPages;
    else if (number==0) {
        ui->current_slide->setUseCache(0);
        ui->notes_widget->setUseCache(0);
        presentationScreen->slide->setUseCache(0);
        maxCacheNumber = 0;
    }
    else
        maxCacheNumber = number;
}

void ControlScreen::setCacheSize(qint64 const size)
{
    // Set maximum memory used for cached pages (in bytes).
    // A negative number is interpreted as infinity.
    if (cacheSize==0) {
        ui->current_slide->setUseCache(0);
        ui->notes_widget->setUseCache(0);
        presentationScreen->slide->setUseCache(0);
    }
    maxCacheSize = size;
}

void ControlScreen::setTocLevel(quint8 const level)
{
    // Set maximum level of sections / subsections shown in the table of contents.
    if (level<1) {
        qWarning() << "toc-depth set to minimum value 1";
        tocBox->setUnfoldLevel(1);
    }
    else if (level>4) {
        qWarning() << "toc-depth set to maximum value 4";
        tocBox->setUnfoldLevel(4);
    }
    else
        tocBox->setUnfoldLevel(level);
}

void ControlScreen::receiveNewPageNumber(int const pageNumber)
{
    renderPage(pageNumber);
    updateCache();
}

void ControlScreen::receiveDest(QString const& dest)
{
    // Receive a TOC destination and go to the corresponding slide.
    hideToc();
    int const pageNumber = presentation->destToSlide(dest);
    if (pageNumber>=0 && pageNumber<numberOfPages) {
        emit sendNewPageNumber(pageNumber, true);
        renderPage(pageNumber);
        ui->label_timer->continueTimer();
        updateCache();
    }
}

void ControlScreen::receivePageShiftEdit(int const shift)
{
    // Shift the page shown on the control screen without changing the presentation screen.
    if (currentPageNumber + shift >= 0) {
        renderPage(currentPageNumber + shift);
        updateCache();
    }
}

void ControlScreen::receivePreviousSlideEnd()
{
    // Go to the end of the previous real slide (not: previous overlay) on the control screen without changing the presentation screen.
    if (currentPageNumber >= 0) {
        renderPage(notes->getPreviousSlideEnd(currentPageNumber));
        updateCache();
    }
}

void ControlScreen::receiveNextSlideStart()
{
    // Go to the beginning of the next real slide (not: next overlay) on the control screen without changing the presentation screen.
    if (currentPageNumber < numberOfPages - 1) {
        renderPage(notes->getNextSlideIndex(currentPageNumber));
        updateCache();
    }
}

void ControlScreen::adaptPage()
{
    // Synchronize the notes page to the presentation page.
    // This function is called after the page of the presentation is changed.
    ui->label_timer->continueTimer();
    // Go to page shifted relative to the page shown on the presentation screen page.
    if (presentationScreen->slide->getDuration() < 0 || presentationScreen->slide->getDuration() > 0.5) {
        renderPage(presentationScreen->getPageNumber());
        updateCache();
    }
}

void ControlScreen::keyPressEvent(QKeyEvent* event)
{
    // Key codes are given as key + modifiers.
    quint32 const key = quint32(event->key()) + quint32(event->modifiers());
    if (tools.contains(key)) {
        presentationScreen->slide->setTool(tools[key]);
        if (drawSlide != nullptr)
            drawSlide->setTool(tools[key]);
    }
    QMap<quint32, QList<KeyAction>>::iterator map_it = keymap->find(key);
    if (map_it == keymap->end())
        return;
    for (QList<KeyAction>::const_iterator action_it=map_it->cbegin(); action_it!=map_it->cend(); action_it++)
        handleKeyAction(*action_it);
    event->accept();
}

void ControlScreen::handleKeyAction(KeyAction const action)
{
    // Handle any kind of action sent by a key binding or a button.
    switch (action) {
    case KeyAction::Next:
        currentPageNumber = presentationScreen->getPageNumber() + 1;
        emit sendNewPageNumber(currentPageNumber, true);
        if (isVisible())
            showNotes();
        ui->label_timer->continueTimer();
        break;
    case KeyAction::Previous:
        currentPageNumber = presentationScreen->getPageNumber() - 1;
        if (currentPageNumber >= 0) {
            emit sendNewPageNumber(currentPageNumber, false);
            showNotes();
            ui->label_timer->continueTimer();
        }
        else {
            currentPageNumber = 0;
        }
        break;
    case KeyAction::NextNotes:
        if (isVisible()) {
            renderPage(++currentPageNumber);
            showNotes();
        }
        break;
    case KeyAction::PreviousNotes:
        if (isVisible()) {
            if (currentPageNumber > 0)
                renderPage(--currentPageNumber);
            showNotes();
        }
        break;
    case KeyAction::NextSkippingOverlays:
        currentPageNumber = presentation->getNextSlideIndex(presentationScreen->getPageNumber());
        emit sendNewPageNumber(currentPageNumber, true);
        if (isVisible())
            showNotes();
        ui->label_timer->continueTimer();
        break;
    case KeyAction::PreviousSkippingOverlays:
        currentPageNumber = presentation->getPreviousSlideEnd(presentationScreen->getPageNumber());
        emit sendNewPageNumber(currentPageNumber, false);
        if (isVisible())
            showNotes();
        ui->label_timer->continueTimer();
        break;
    case KeyAction::NextNotesSkippingOverlays:
        if (isVisible()) {
            currentPageNumber = presentation->getNextSlideIndex(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
        }
        break;
    case KeyAction::PreviousNotesSkippingOverlays:
        if (isVisible()) {
            if (currentPageNumber > 0) {
                currentPageNumber = presentation->getPreviousSlideEnd(currentPageNumber);
                renderPage(currentPageNumber);
            }
            showNotes();
        }
        break;
    case KeyAction::PreviousNoTransition:
        currentPageNumber = presentationScreen->getPageNumber() - 1;
        presentationScreen->slide->disableTransitions();
        if (currentPageNumber >= 0) {
            emit sendNewPageNumber(currentPageNumber, false);
            showNotes();
            ui->label_timer->continueTimer();
        }
        else
            currentPageNumber = 0;
        presentationScreen->slide->enableTransitions();
        break;
    case KeyAction::NextNoTransition:
        currentPageNumber = presentationScreen->getPageNumber() + 1;
        presentationScreen->slide->disableTransitions();
        emit sendNewPageNumber(currentPageNumber, true);
        presentationScreen->slide->enableTransitions();
        if (isVisible())
            showNotes();
        ui->label_timer->continueTimer();
        break;
    case KeyAction::Update:
        currentPageNumber = presentationScreen->getPageNumber();
        emit sendNewPageNumber(currentPageNumber, true); // TODO: what happens to duration if page is updated?
        if (isVisible())
            showNotes();
        ui->label_timer->continueTimer();
        break;
    case KeyAction::LastPage:
        currentPageNumber = numberOfPages - 1;
        emit sendNewPageNumber(currentPageNumber, false);
        if (isVisible())
            showNotes();
        break;
    case KeyAction::FirstPage:
        currentPageNumber = 0;
        emit sendNewPageNumber(currentPageNumber, true);
        if (isVisible())
            showNotes();
        break;
    case KeyAction::UpdateCache:
        updateCache();
        break;
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    case KeyAction::StartEmbeddedCurrentSlide:
        presentationScreen->slide->startAllEmbeddedApplications(presentationScreen->getPageNumber());
        //ui->notes_widget->startAllEmbeddedApplications(currentPageNumber);
        break;
    case KeyAction::StartAllEmbedded:
        startAllEmbeddedApplications();
        break;
    case KeyAction::CloseEmbeddedCurrentSlide:
    {
        presentationScreen->slide->closeEmbeddedApplications(presentationScreen->getPageNumber());
        ui->notes_widget->closeEmbeddedApplications(presentationScreen->getPageNumber());
    }
        break;
    case KeyAction::CloseAllEmbedded:
        presentationScreen->slide->closeAllEmbeddedApplications();
        ui->notes_widget->closeAllEmbeddedApplications();
        break;
#endif
    case KeyAction::GoToPage:
        if (isVisible()) {
            showNotes();
            ui->text_current_slide->setFocus();
        }
        break;
    case KeyAction::PlayMultimedia:
        emit playMultimedia();
        break;
    case KeyAction::PauseMultimedia:
        emit pauseMultimedia();
        break;
    case KeyAction::PlayPauseMultimedia:
        {
            bool running = ui->notes_widget->hasActiveMultimediaContent() || presentationScreen->slide->hasActiveMultimediaContent();
            if (running)
                emit pauseMultimedia();
            else
                emit playMultimedia();
        }
        break;
    case KeyAction::ToggleMuteAll:
    {
        bool mute = !presentationScreen->slide->isMuted();
        presentationScreen->slide->setMuted(mute);
        ui->notes_widget->setMuted(mute);
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(mute);
        break;
    }
    case KeyAction::ToggleMuteNotes:
        ui->notes_widget->setMuted(!ui->notes_widget->isMuted());
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(ui->notes_widget->isMuted());
        break;
    case KeyAction::ToggleMutePresentation:
        presentationScreen->slide->setMuted(!presentationScreen->slide->isMuted());
        break;
    case KeyAction::MuteAll:
        presentationScreen->slide->setMuted(true);
        ui->notes_widget->setMuted(true);
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(true);
        break;
    case KeyAction::MuteNotes:
        ui->notes_widget->setMuted(true);
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(true);
        break;
    case KeyAction::MutePresentation:
        presentationScreen->slide->setMuted(true);
        break;
    case KeyAction::UnmuteAll:
        presentationScreen->slide->setMuted(false);
        ui->notes_widget->setMuted(false);
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(false);
        break;
    case KeyAction::UnmuteNotes:
        ui->notes_widget->setMuted(false);
        if (drawSlide != nullptr && drawSlide != ui->notes_widget)
            drawSlide->setMuted(false);
        break;
    case KeyAction::UnmutePresentation:
        presentationScreen->slide->setMuted(false);
        break;
    case KeyAction::ShowCursor:
        presentationScreen->slide->showPointer();
        break;
    case KeyAction::HideCursor:
        presentationScreen->slide->hidePointer();
        break;
    case KeyAction::ToggleCursor:
        presentationScreen->slide->togglePointerVisibility();
        break;
    case KeyAction::PlayPauseTimer:
        ui->label_timer->toggleTimer();
        break;
    case KeyAction::ContinueTimer:
        ui->label_timer->continueTimer();
        break;
    case KeyAction::PauseTimer:
        ui->label_timer->pauseTimer();
        break;
    case KeyAction::ResetTimer:
        ui->label_timer->resetTimer();
        break;
    case KeyAction::ShowTOC:
        if (isVisible())
            showToc();
        break;
    case KeyAction::HideTOC:
        if (isVisible())
            hideToc();
        break;
    case KeyAction::ToggleTOC:
        if (tocBox->isVisible())
            hideToc();
        else if (isVisible())
            showToc();
        break;
    case KeyAction::ShowOverview:
        if (isVisible())
            showOverview();
        break;
    case KeyAction::HideOverview:
        if (isVisible())
            hideOverview();
        break;
    case KeyAction::ToggleOverview:
        if (overviewBox->isVisible())
            hideOverview();
        else if (isVisible())
            showOverview();
        break;
    case KeyAction::HideDrawSlide:
        if (isVisible())
            hideDrawSlide();
        break;
    case KeyAction::Reload:
        reloadFiles();
        break;
    case KeyAction::SyncFromControlScreen:
        if (isVisible()) {
            if (presentationScreen->slide->pageNumber() != currentPageNumber)
                emit sendNewPageNumber(currentPageNumber, true); // TODO: make this configurable (true/false)?
            showNotes();
            updateCache();
            ui->label_timer->continueTimer();
        }
        break;
    case KeyAction::SyncFromPresentationScreen:
        if (isVisible() && presentationScreen->getPageNumber() != currentPageNumber) {
            currentPageNumber = presentationScreen->getPageNumber();
            renderPage(currentPageNumber);
            updateCache();
        }
        break;
    case KeyAction::FullScreen:
        if (this->windowState() == Qt::WindowFullScreen)
            showNormal();
        else
            showFullScreen();
        break;
    case KeyAction::Quit:
        emit sendCloseSignal();
        close();
        break;
    case KeyAction::ClearAnnotations:
        presentationScreen->slide->clearPageAnnotations();
        if (drawSlide != nullptr)
            drawSlide->clearPageAnnotations();
        break;
    case KeyAction::DrawNone:
        presentationScreen->slide->setTool(NoTool);
        if (drawSlide != nullptr)
            drawSlide->setTool(NoTool);
        break;
    case KeyAction::DrawEraser:
        presentationScreen->slide->setTool(Eraser, QColor());
        if (drawSlide != nullptr)
            drawSlide->setTool(Eraser, QColor());
        break;
    case KeyAction::DrawMode:
        if (isVisible())
            showDrawSlide();
        break;
    case KeyAction::ToggleDrawMode:
        if (isVisible()) {
            if (drawSlide == nullptr || drawSlide->isHidden())
                showDrawSlide();
            else if (drawSlide != ui->notes_widget)
                hideDrawSlide();
        }
        break;
    case KeyAction::UndoDrawing:
        presentationScreen->slide->undoPath();
        break;
    case KeyAction::RedoDrawing:
        presentationScreen->slide->redoPath();
        break;
    case KeyAction::SaveDrawings:
        {
            QString const savePath = QFileDialog::getSaveFileName(this, "Save drawings");
            if (!savePath.isEmpty())
                presentationScreen->slide->saveDrawings(savePath, notes->getPath());
        }
        break;
    case KeyAction::LoadDrawings:
        {
            QString const loadPath = QFileDialog::getOpenFileName(this, "Load drawings");
            if (!loadPath.isEmpty())
                presentationScreen->slide->loadDrawings(loadPath);
        }
        break;
    case NoAction:
        break;
    default:
        ColoredDrawTool const tool = actionToToolMap.value(action, {InvalidTool, QColor()});
        if (tool.tool != InvalidTool) {
            presentationScreen->slide->setTool(tool);
            if (drawSlide != nullptr)
                drawSlide->setTool(tool);
        }
        break;
    }
}

#ifdef EMBEDDED_APPLICATIONS_ENABLED
void ControlScreen::startAllEmbeddedApplications()
{
    // Start all embedded applications of the presentation on all pages.
    qDebug() << "Starting all embedded applications on all pages.";
    for (int i=0; i<presentation->getDoc()->numPages(); i++) {
        presentationScreen->slide->initEmbeddedApplications(i);
        presentationScreen->slide->startAllEmbeddedApplications(i);
    }
}
#endif

void ControlScreen::resizeEvent(QResizeEvent* event)
{
    // When the control screen window is resized, the sizes of the page labels change and the cached pages become useless.
    // Stop rendering to cache, delete cached pages for the control screen labels and reset cached region.
    cacheThread->requestInterruption();
    cacheTimer->stop();
    recalcLayout(currentPageNumber);
    oldSize = event->size();
    first_cached = presentationScreen->getPageNumber();
    last_cached = first_cached-1;
    first_delete = 0;
    last_delete = numberOfPages-1;
    ui->notes_widget->clearCache();
    ui->current_slide->clearCache();
    ui->next_slide->clearCache();
    overviewBox->setOutdated();
    // Render current page.
    ui->notes_widget->renderPage(ui->notes_widget->pageNumber(), false);
    ui->current_slide->renderPage(ui->current_slide->pageNumber());
    ui->next_slide->renderPage(ui->next_slide->pageNumber());
}

void ControlScreen::clearPresentationCache()
{
    // Stop rendering to cache and reset cached region
    cacheThread->requestInterruption();
    cacheTimer->stop();
    first_cached = presentationScreen->getPageNumber();
    last_cached = first_cached-1;
    first_delete = 0;
    last_delete = numberOfPages-1;
}

void ControlScreen::setColor(const QColor bgColor, const QColor textColor)
{
    // Set background and text color for the control screen.
    QPalette newPalette(palette());
    newPalette.setColor(QPalette::Background, bgColor);
    newPalette.setColor(QPalette::Base, bgColor);
    newPalette.setColor(QPalette::Text, textColor);
    newPalette.setColor(QPalette::WindowText, textColor);
    setPalette(newPalette);
    ui->text_current_slide->setPalette(newPalette);
}

void ControlScreen::setPresentationColor(const QColor color)
{
    // set background color for the presentation screen.
    QPalette newPalette(presentationScreen->palette());
    newPalette.setColor(QPalette::Background, color);
    newPalette.setColor(QPalette::Base, color);
    presentationScreen->setPalette(newPalette);
}

void ControlScreen::setScrollDelta(const int scrollDelta)
{
    this->scrollDelta = scrollDelta;
    presentationScreen->setScrollDelta(scrollDelta);
}

void ControlScreen::setForceTouchpad()
{
    forceIsTouchpad = true;
    presentationScreen->setForceTouchpad();
}

void ControlScreen::wheelEvent(QWheelEvent* event)
{
    // Handle mouse wheel or touch pad scrolling events.

    // Change the signs in the beginning, this makes the rest less confusing.
    int deltaPix = -event->pixelDelta().y();
    int deltaAngle = -event->angleDelta().y();
    int deltaPages;
    // If a touch pad was used for scrolling:
    if (deltaPix != 0) {
        scrollState += deltaPix;
        deltaPages = scrollState / scrollDelta;
        if (deltaPages<0)
            deltaPages++;
        scrollState -= scrollDelta*deltaPages;
    }
    else if (forceIsTouchpad) {
        scrollState += deltaAngle;
        deltaPages = scrollState / scrollDelta;
        if (deltaPages<0)
            deltaPages++;
        scrollState -= scrollDelta*deltaPages;
    }
    // If a mouse wheel was used for scrolling:
    else {
        if (deltaAngle > 120)
            deltaPages = deltaAngle / 120;
        else if (deltaAngle > 0)
            deltaPages = 1;
        else if (deltaAngle < -120)
            deltaPages = -((-deltaAngle) / 120);
        else if (deltaAngle < 0)
            deltaPages = -1;
        else
            deltaPages = 0;
    }
    if (deltaPages + currentPageNumber < 0) {
        if (currentPageNumber != 0)
            renderPage(0);
    }
    else if (deltaPages != 0) {
        renderPage(currentPageNumber + deltaPages);
        updateCache();
    }
    event->accept();
}

#ifdef EMBEDDED_APPLICATIONS_ENABLED
void ControlScreen::setEmbedFileList(const QStringList &files)
{
    // Set list of files, which should be be executed as embedded widgets if they are linked in the PDF.
    ui->notes_widget->setEmbedFileList(files);
    presentationScreen->slide->setEmbedFileList(files);
}

void ControlScreen::setPid2WidConverter(QString const &program)
{
    // Set an external program for converting process IDs to window IDs.
    QFileInfo fileinfo = QFileInfo(program);
    if (fileinfo.isFile() && fileinfo.isExecutable()) {
        ui->notes_widget->setPid2Wid(program);
        presentationScreen->slide->setPid2Wid(program);
    }
    else
        qCritical() << "Can't use program: not a file or not executable." << program;
}
#endif

void ControlScreen::setUrlSplitCharacter(QString const &splitCharacter)
{
    // Set a character, which splits links to files into file paths and arguments.
    ui->notes_widget->setUrlSplitCharacter(splitCharacter);
    presentationScreen->slide->setUrlSplitCharacter(splitCharacter);
}

void ControlScreen::showToc()
{
    // Show table of contents on the control screen (above the notes label).
    hideOverview();
    if (tocBox->needUpdate())
        tocBox->createToc(presentation->getToc());
    if (!tocBox->hasToc()) {
        qWarning() << "This document does not contain a table of contents";
        return;
    }
    if (!this->isActiveWindow())
        this->activateWindow();
    ui->notes_widget->hide();
    tocBox->show();
    tocBox->raise();
    tocBox->setFocus();
}

void ControlScreen::hideToc()
{
    // Hide table of contents
    tocBox->hide();
    ui->notes_widget->show();
    ui->notes_widget->setFocus();
}

void ControlScreen::showNotes()
{
    // Equivalent to hideToc(); hideOverview();
    tocBox->hide();
    overviewBox->hide();
    if (drawSlide == nullptr || drawSlide->isHidden()) {
        ui->notes_widget->show();
        ui->notes_widget->setFocus();
    }
}

void ControlScreen::showOverview()
{
    // Show overview on the control screen (above the notes label).
    hideToc();
    if (overviewBox->needsUpdate()) {
        cacheThread->requestInterruption();
        cacheTimer->stop();
        overviewBox->create(presentation, overviewColumns, pagePart);
    }
    if (!this->isActiveWindow())
        this->activateWindow();
    ui->notes_widget->hide();
    overviewBox->show();
    overviewBox->raise();
    overviewBox->setFocused(presentationScreen->getPageNumber());
}

void ControlScreen::hideOverview()
{
    // Hide overview
    overviewBox->hide();
    ui->notes_widget->show();
    ui->notes_widget->setFocus();
}

void ControlScreen::setRenderer(QStringList command)
{
    // Set a command for an external renderer.
    // This function also checks whether the command uses the arguments %file, %page, %width and %height

    if (command.size() == 1) {
        if (command[0]=="poppler") {
            cacheThread->setRenderer(Renderer::poppler);
            return;
        }
        if (command[0]=="custom") {
            if (cacheThread->hasRenderCommand()) {
                cacheThread->setRenderer(Renderer::custom);
                return;
            }
            qCritical() << "Ignored request to use undefined custom renderer";
        }
        else
            qCritical() << "Ignored request to use custom renderer without arguments";
        throw 1;
    }
    QString program = command[0];
    command.removeFirst();
    if (command.filter("%file").isEmpty()) {
        qCritical() << "Ignored request to use custom renderer without %file in arguments";
        throw 2;
    }
    if (command.filter("%page").isEmpty()) {
        qCritical() << "Ignored request to use custom renderer without %page in arguments";
        throw 3;
    }
    if (command.filter("%width").isEmpty())
        qWarning() << "Custom renderer does not use %width in arguments";
    if (command.filter("%height").isEmpty())
        qWarning() << "Custom renderer does not use %height in arguments";
    cacheThread->setCustomRenderer(program, presentation->getPath(), notes->getPath(), command);
    presentationScreen->slide->setUseCache(2);
    ui->notes_widget->setUseCache(2);
    qDebug() << "set render command to" << program << presentation->getPath() << notes->getPath() << command;
    return;
}

void ControlScreen::reloadFiles()
{
    // Reload the pdf files if they have been updated.

    // Stop the cache management.
    cacheThread->requestInterruption();
    cacheTimer->stop();
    // Wait until the cacheThread finishes.
    // Deleting the old documents while cacheThread is still running will usually lead to a segmentation fault.
    if (!cacheThread->wait(10000)) {
        qCritical() << "Could not stop a running cache process. Reloading files failed. You can try again.";
        return;
        //cacheThread->terminate();
        //if (!cacheThread->wait(10000))
        //    return;
    }

    bool change = false;
    bool sameFile = presentation->getPath()==notes->getPath();
    // Reload notes file
    if (notes->loadDocument()) {
        qInfo() << "Reloading notes file";
        change = true;
        ui->notes_widget->clearAll();
        recalcLayout(currentPageNumber);
    }
    // Reload presentation file
    if ((sameFile && change) || (!sameFile && presentation->loadDocument())) {
        qInfo() << "Reloading presentation file";
        change = true;
        bool unlimitedCache = numberOfPages==maxCacheNumber;
        numberOfPages = presentation->getDoc()->numPages();
        if (unlimitedCache)
            maxCacheNumber = numberOfPages;
        presentationScreen->updatedFile();
        presentationScreen->renderPage(presentationScreen->slide->pageNumber(), false);
        ui->current_slide->clearAll();
        ui->next_slide->clearAll();
        // Hide TOC and overview and set them outdated
        showNotes();
        tocBox->setOutdated();
        tocBox->createToc(presentation->getToc());
        overviewBox->setOutdated();
    }
    // If one of the two files has changed: Reset cache region and render pages on control screen.
    if (change) {
        first_cached = currentPageNumber;
        last_cached = first_cached-1;
        first_delete = 0;
        last_delete = numberOfPages-1;
        renderPage(currentPageNumber);
        ui->text_number_slides->setText(QString::number(numberOfPages));
        ui->text_current_slide->setNumberOfPages(numberOfPages);
    }
    updateCache();
}

void ControlScreen::setKeyMap(QMap<quint32, QList<KeyAction>>* keymap)
{
    // Set the key bindings
    delete this->keymap;
    this->keymap = keymap;
}

void ControlScreen::setKeyMapItem(quint32 const key, KeyAction const action)
{
    // Add an action to a key
    QMap<quint32, QList<KeyAction>>::iterator map_it = keymap->find(key);
    if (map_it==keymap->end())
        keymap->insert(key, {action});
    else
        map_it->append(action);
}

void ControlScreen::setTimerMap(QMap<int, QTime> &timeMap)
{
    // Set times per slide for timer color change
    ui->label_timer->setTimeMap(timeMap);
    ui->label_timer->setPage(presentationScreen->getPageNumber());
}

void ControlScreen::showDrawSlide()
{
    // Draw slide and tool selector
    if (drawSlide == nullptr) {
        drawSlide = new DrawSlide(this);
        drawSlide->setDoc(presentation);
        drawSlide->setFocusPolicy(Qt::ClickFocus);
        drawSlide->setUseCache(0);
        connect(ui->tool_selector, &ToolSelector::sendNewTool, drawSlide, QOverload<const ColoredDrawTool>::of(&DrawSlide::setTool));
        connect(drawSlide, &DrawSlide::pathsChangedQuick, presentationScreen->slide, &DrawSlide::setPathsQuick);
        connect(presentationScreen->slide, &DrawSlide::pathsChangedQuick, drawSlide, &DrawSlide::setPathsQuick);
        connect(drawSlide, &DrawSlide::pathsChanged, presentationScreen->slide, &DrawSlide::setPaths);
        connect(presentationScreen->slide, &DrawSlide::pathsChanged, drawSlide, &DrawSlide::setPaths);
        connect(drawSlide, &DrawSlide::pointerPositionChanged, presentationScreen->slide, &DrawSlide::setPointerPosition);
        connect(presentationScreen->slide, &DrawSlide::pointerPositionChanged, drawSlide, &DrawSlide::setPointerPosition);
        connect(drawSlide, &DrawSlide::sendRelax, presentationScreen->slide, &DrawSlide::relax);
        connect(presentationScreen->slide, &DrawSlide::sendRelax, drawSlide, &DrawSlide::relax);
        connect(drawSlide, &DrawSlide::sendUpdateEnlargedPage, presentationScreen->slide, &DrawSlide::updateEnlargedPage);
        connect(presentationScreen->slide, &DrawSlide::sendUpdateEnlargedPage, drawSlide, &DrawSlide::updateEnlargedPage);
        connect(drawSlide, &DrawSlide::sendUpdatePathCache, presentationScreen->slide, &DrawSlide::updatePathCache);
        connect(presentationScreen->slide, &DrawSlide::sendUpdatePathCache, drawSlide, &DrawSlide::updatePathCache);
        connect(drawSlide, &PreviewSlide::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPage);
        connect(drawSlide, &PreviewSlide::sendNewPageNumber, this, [&](int const pageNumber){renderPage(pageNumber);});
        connect(drawSlide, &MediaSlide::requestMultimediaSliders, this, &ControlScreen::interconnectMultimediaSliders);
        connect(drawSlide, &MediaSlide::sendPlayVideo,  presentationScreen->slide, &MediaSlide::playVideo);
        connect(drawSlide, &MediaSlide::sendPauseVideo, presentationScreen->slide, &MediaSlide::pauseVideo);
        connect(presentationScreen->slide, &MediaSlide::sendPlayVideo,  drawSlide, &MediaSlide::playVideo);
        connect(presentationScreen->slide, &MediaSlide::sendPauseVideo, drawSlide, &MediaSlide::pauseVideo);
    }
    else if (drawSlide == ui->notes_widget)
        return;

    drawSlide->setMuted(ui->notes_widget->isMuted());
    drawSlide->setGeometry(ui->notes_widget->rect());
    recalcLayout(currentPageNumber);
    drawSlide->renderPage(presentationScreen->slide->pageNumber(), false);
    drawSlide->setTool(presentationScreen->slide->getTool());
    ui->notes_widget->hide();
    drawSlide->show();
    drawSlide->setFocus();
    qint16 const sx=presentationScreen->slide->getXshift(), sy=presentationScreen->slide->getYshift();
    double const res = presentationScreen->slide->getResolution();
    double const scale = drawSlide->getResolution() / res;
    drawSlide->setSize(Pen, static_cast<quint16>(scale*presentationScreen->slide->getSize(Pen)+0.5));
    drawSlide->setSize(Pointer, static_cast<quint16>(scale*presentationScreen->slide->getSize(Pointer)+0.5));
    drawSlide->setSize(Highlighter, static_cast<quint16>(scale*presentationScreen->slide->getSize(Highlighter)+0.5));
    drawSlide->setSize(Torch, static_cast<quint16>(scale*presentationScreen->slide->getSize(Torch)));
    drawSlide->setSize(Magnifier, static_cast<quint16>(scale*presentationScreen->slide->getSize(Magnifier)));
    QString const label = presentationScreen->slide->getPage()->label();
    drawSlide->setPaths(label, presentationScreen->slide->getPaths()[label], sx, sy, res);
    drawSlide->update();
    renderPage(currentPageNumber);
    drawSlide->setAutostartDelay(presentationScreen->slide->getAutostartDelay());
}

void ControlScreen::hideDrawSlide()
{
    // TODO: fix this: switching while a video is playing leads to broken VideoWidgets
    if (drawSlide != nullptr && drawSlide != ui->notes_widget) {
        drawSlide->hide();
        delete drawSlide;
        drawSlide = nullptr;
    }
    recalcLayout(currentPageNumber);
    ui->notes_widget->show();
    renderPage(currentPageNumber);
    ui->notes_widget->showAllWidgets(); // This function does not exactly what it should do.
}

ToolSelector* ControlScreen::getToolSelector()
{
     return ui->tool_selector;
}

void ControlScreen::setMagnification(const qreal mag)
{
    presentationScreen->slide->setMagnification(mag);
    if (drawSlide != nullptr)
        drawSlide->setMagnification(mag);
}

void ControlScreen::setAutostartDelay(const double timeout)
{
    // Autostart of media on the control screen can be enabled by uncommenting the following line.
    //ui->notes_widget->setAutostartDelay(timeout);
    presentationScreen->slide->setAutostartDelay(timeout);
    if (drawSlide != nullptr)
        drawSlide->setAutostartDelay(timeout);
}

MediaSlide* ControlScreen::getNotesSlide()
{
    return ui->notes_widget;
}
