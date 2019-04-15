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
        if (!notesPath.isEmpty()) {
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
    presentationScreen->setKeyMap(keymap);

    // Load notes pdf
    if (notesPath.isNull() || notesPath.isEmpty()) {
        notes = presentation;
        setWindowTitle("BeamerPresenter: " + presentationPath);
    }
    else {
        notes = new PdfDoc(notesPath);
        notes->loadDocument();
        if (notes->getDoc() == nullptr) {
            qCritical() << "File could not be opened as PDF: " << notesPath;
            notes = presentation;
            setWindowTitle("BeamerPresenter: " + presentationPath);
        }
        else
            setWindowTitle("BeamerPresenter: " + notesPath);
    }

    // tocBox will be used to show the table of contents on the control screen
    tocBox = new TocBox(this);
    tocBox->setGeometry(ui->notes_label->geometry());
    tocBox->hide();

    // overviewBox will be used to show an overview of all slides on the control screen
    overviewBox = new OverviewBox(this);
    overviewBox->setGeometry(ui->notes_label->geometry());
    overviewBox->hide();

    // Set up the widgets
    ui->text_number_slides->setText(QString::number(numberOfPages));
    ui->text_current_slide->setNumberOfPages(numberOfPages);
    ui->notes_label->setPresentationStatus(false);
    ui->current_slide_label->setShowMultimedia(false);
    ui->next_slide_label->setShowMultimedia(false);
    ui->next_slide_label->setUseCache(false);
    ui->notes_label->setFocus();


    // Page requests from the labels:
    // These are emitted if links are clicked.
    // These events are send to ControlScreen and PresentationScreen
    connect(ui->notes_label,         &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->notes_label,         &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);

    connect(ui->notes_label,         &PageLabel::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(ui->current_slide_label, &PageLabel::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(ui->next_slide_label,    &PageLabel::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);
    connect(presentationScreen->getLabel(), &PageLabel::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);

    connect(ui->notes_label,         &PageLabel::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->current_slide_label, &PageLabel::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->next_slide_label,    &PageLabel::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(presentationScreen->getLabel(), &PageLabel::sendShowFullscreen, this, &ControlScreen::showFullScreen);
    connect(ui->notes_label,         &PageLabel::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(ui->current_slide_label, &PageLabel::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(ui->next_slide_label,    &PageLabel::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);
    connect(presentationScreen->getLabel(), &PageLabel::sendShowFullscreen, presentationScreen, &PresentationScreen::showFullScreen);

    // Navigation signals emitted by PresentationScreen:
    connect(presentationScreen, &PresentationScreen::sendPageShift,     this, &ControlScreen::receivePageShiftReturn);
    connect(presentationScreen, &PresentationScreen::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);

    // Other signals emitted by PresentationScreen
    connect(presentationScreen, &PresentationScreen::sendKeyEvent,    this, &ControlScreen::keyPressEvent);
    connect(presentationScreen, &PresentationScreen::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);
    connect(presentationScreen, SIGNAL(sendUpdateCache()), this, SLOT(updateCache()));
    connect(presentationScreen->getLabel(), &PageLabel::requestMultimediaSliders, this, &ControlScreen::addMultimediaSliders);
    connect(presentationScreen, &PresentationScreen::focusPageNumberEdit, this, &ControlScreen::focusPageNumberEdit);

    // Signals sent back to PresentationScreen
    connect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(this, &ControlScreen::sendCloseSignal,   presentationScreen, &PresentationScreen::receiveCloseSignal);
    connect(ui->notes_label, &PageLabel::sendCloseSignal, presentationScreen, &PresentationScreen::receiveCloseSignal);
    connect(presentationScreen->getLabel(), &PageLabel::sendCloseSignal, presentationScreen, &PresentationScreen::receiveCloseSignal);
    connect(ui->notes_label, &PageLabel::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);
    connect(presentationScreen->getLabel(), &PageLabel::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);

    ui->label_timer->setTimerWidget(ui->edit_timer);
    // Signals emitted by the timer
    connect(ui->label_timer, &Timer::sendAlert,   this, &ControlScreen::receiveTimerAlert);
    connect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    connect(ui->label_timer, &Timer::sendEscape,  this, &ControlScreen::resetFocus);
    // Signals sent back to the timer
    connect(this, &ControlScreen::sendTimerString,     ui->label_timer, &Timer::receiveTimerString);
    connect(this, &ControlScreen::sendTimeoutInterval, ui->label_timer, &Timer::receiveTimeoutInterval);

    // Signals sent to the page labels
    // Autostart of media on the control screen can be enabled by uncommenting the following lines.
    //connect(this, &ControlScreen::sendAutostartDelay, ui->notes_label, &PageLabel::setAutostartDelay);
    connect(this, &ControlScreen::sendAutostartDelay, presentationScreen->getLabel(), &PageLabel::setAutostartDelay);
    //connect(this, &ControlScreen::sendAutostartEmbeddedDelay, ui->notes_label, &PageLabel::setAutostartEmbeddedDelay);
    connect(this, &ControlScreen::sendAutostartEmbeddedDelay, presentationScreen->getLabel(), &PageLabel::setAutostartEmbeddedDelay);
    //connect(this, &ControlScreen::playMultimedia,     ui->notes_label, &PageLabel::startAllMultimedia);
    connect(this, &ControlScreen::playMultimedia,     presentationScreen->getLabel(), &PageLabel::startAllMultimedia);
    //connect(this, &ControlScreen::pauseMultimedia,    ui->notes_label, &PageLabel::pauseAllMultimedia);
    connect(this, &ControlScreen::pauseMultimedia,    presentationScreen->getLabel(), &PageLabel::pauseAllMultimedia);
    connect(this, &ControlScreen::sendAnimationDelay, presentationScreen->getLabel(), &PageLabel::setAnimationDelay);
    connect(this, &ControlScreen::togglePointerVisibilitySignal, presentationScreen->getLabel(), &PageLabel::togglePointerVisibility);

    // Signals emitted by the page number editor
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit,   this, &ControlScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit,    this, &ControlScreen::receivePageShiftEdit);
    connect(ui->text_current_slide, &PageNumberEdit::sendNextSlideStart,   this, &ControlScreen::receiveNextSlideStart);
    connect(ui->text_current_slide, &PageNumberEdit::sendPreviousSlideEnd, this, &ControlScreen::receivePreviousSlideEnd);
    connect(ui->text_current_slide, &PageNumberEdit::sendEscape,           this, &ControlScreen::resetFocus);

    // Cache handling
    connect(cacheTimer, SIGNAL(timeout()), this, SLOT(updateCacheStep()));
    cacheThread->setLabels(presentationScreen->getLabel(), ui->notes_label, ui->current_slide_label);
    connect(cacheThread, &CacheUpdateThread::resultsReady, this, &ControlScreen::receiveCache);
    connect(presentationScreen, &PresentationScreen::clearPresentationCacheRequest, this, &ControlScreen::clearPresentationCache);

    // TOC
    connect(tocBox, &TocBox::sendDest, this, &ControlScreen::receiveDest);

    // Overview box
    connect(overviewBox, &OverviewBox::sendPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
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
    ui->notes_label->disconnect();
    ui->current_slide_label->disconnect();
    ui->next_slide_label->disconnect();
    ui->label_timer->disconnect();
    ui->text_current_slide->disconnect();
    presentationScreen->getLabel()->disconnect();
    presentationScreen->disconnect();
    if (notes != presentation)
        delete notes;
    delete keymap;
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
            ui->notes_label->setPagePart(FullPage);
            break;
        case LeftHalf:
            cacheThread->setPagePart(LeftHalf);
            ui->notes_label->setPagePart(RightHalf);
            break;
        case RightHalf:
            cacheThread->setPagePart(RightHalf);
            ui->notes_label->setPagePart(LeftHalf);
            break;
    }
    ui->current_slide_label->setPagePart(pagePart);
    ui->next_slide_label->setPagePart(pagePart);
    presentationScreen->getLabel()->setPagePart(pagePart);
}

void ControlScreen::recalcLayout(const int pageNumber)
{
    // Calculate a good size for the notes side bar
    double screenRatio = double(height()) / width();
    if (screenRatio > 1)
        screenRatio = 1.;
    QSize notesSize = notes->getPageSize(pageNumber);
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
    ui->notes_label->setGeometry(0, 0, width()-sideWidth, height());
    ui->current_slide_label->setMaximumWidth(sideWidth);
    ui->next_slide_label->setMaximumWidth(sideWidth);
    ui->gridLayout->setColumnStretch(0, width()-sideWidth);
    ui->gridLayout->setColumnStretch(1, sideWidth);
    tocBox->setGeometry(int(0.1*(width()-sideWidth)), 0, int(0.8*(width()-sideWidth)), height());
    overviewBox->setGeometry(0, 0, width()-sideWidth, height());
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
    presentationScreen->getLabel()->setMultimediaSliders(sliderList);
}

void ControlScreen::resetFocus()
{
    // Make sure that the control screen shows the same page as the presentation screen and set focus to the notes label.
    if (currentPageNumber != presentationScreen->getPageNumber())
        renderPage(presentationScreen->getPageNumber());
    ui->notes_label->setFocus();
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

void ControlScreen::renderPage(int const pageNumber)
{
    // Update all page labels on the control screen to show the given page number.
    // This uses cached pages if such are available.

    // Negative page numbers are interpreted as signal for going to the last page.
    if (pageNumber < 0 || pageNumber >= numberOfPages)
        currentPageNumber = numberOfPages - 1;
    else
        currentPageNumber = pageNumber;

    // Recalculate layout the window size has changed.
    if (size() != oldSize) {
        recalcLayout(currentPageNumber);
        oldSize = size();
    }

    // Update the notes label
    // The note document could have fewer pages than the presentation document:
    if (currentPageNumber < notes->getDoc()->numPages())
        ui->notes_label->renderPage(notes->getPage(currentPageNumber), false);
    else
        qWarning() << "Reached the end of the note file.";

    // Update the current and next slide label.
    // If we have not reached the last page (there exists a next page):
    if (currentPageNumber + 1 < presentation->getDoc()->numPages()) {
        ui->current_slide_label->renderPage(presentation->getPage(currentPageNumber), false);
        if (maxCacheSize!=0 && maxCacheNumber!=0) {
            // Render the next slide to the current slide label's cache
            ui->current_slide_label->updateCache(presentation->getPage(currentPageNumber+1));
            // Now show the cached page on the next slide label
            QPixmap const* pixmap = ui->current_slide_label->getCache(currentPageNumber+1);
            ui->next_slide_label->renderPage(presentation->getPage(currentPageNumber+1), false, pixmap);
            delete pixmap;
        }
        else // No cache: this is inefficient.
            ui->next_slide_label->renderPage(presentation->getPage(currentPageNumber+1));
    }
    else { // If we have not reached the last page (there is no next page)
        // Get page and pixmap for current page.
        Poppler::Page* page = presentation->getPage(currentPageNumber);
        QPixmap const* pixmap = ui->current_slide_label->getCache(currentPageNumber);
        // Show the page on the current and next slide label.
        if (pixmap->isNull()) {
            // The page was not cached. Render it on current slide label.
            ui->current_slide_label->renderPage(page);
            delete pixmap;
            pixmap = ui->current_slide_label->getCache(currentPageNumber);
        }
        else
            ui->current_slide_label->renderPage(page, false, pixmap);
        ui->next_slide_label->renderPage(page, false, pixmap);
        delete pixmap;
    }
    // Update the page number
    ui->text_current_slide->setText(QString::number(currentPageNumber+1));
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
    cacheNumber = presentationScreen->getLabel()->getCacheNumber();
    if (cacheNumber == numberOfPages)
        // All slides are cached
        return;
    // Size of currently cached slides in memory (set to -infinity if it should be ignored)
    if (maxCacheSize > 0)
        cacheSize = presentationScreen->getLabel()->getCacheSize()
             + ui->notes_label->getCacheSize()
             + ui->current_slide_label->getCacheSize();
    else
        // This is approximately -infinity and means that the cache size is unlimited:
        cacheSize = -4294967296; // -4GiB

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
    * 5. receiveCache saves the cached pages to the pagelabels' cache.
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
            delta = presentationScreen->getLabel()->clearCachePage(last_delete);
            cacheSize -= ui->notes_label->clearCachePage(last_delete)
                    + ui->current_slide_label->clearCachePage(last_delete);
            if (delta != 0) {
                cacheNumber--;
                cacheSize -= delta;
                qDebug() << "Freed last page" << last_delete << ". Cache size" << cacheSize << "B";
            }
            last_delete--;
            last_cached = last_delete > last_cached ? last_cached : last_delete;
        }
        else {
            delta = presentationScreen->getLabel()->clearCachePage(first_delete);
            cacheSize -= ui->notes_label->clearCachePage(first_delete)
                    + ui->current_slide_label->clearCachePage(first_delete);
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
            if (!presentationScreen->getLabel()->cacheContains(first_cached-1)) {
                cacheTimer->stop();
                cacheThread->setPages(presentation->getPage(first_cached-1), notes->getPage(first_cached-1));
                cacheThread->start();
            }
            else {
                first_cached--;
                cacheSize += ui->notes_label->updateCache(notes->getPage(first_cached))
                             + ui->current_slide_label->updateCache(presentation->getPage(first_cached));
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
        if (!presentationScreen->getLabel()->cacheContains(last_cached+1)) {
            cacheTimer->stop();
            cacheThread->setPages(presentation->getPage(last_cached+1), notes->getPage(last_cached+1));
            cacheThread->start();
        }
        else {
            // If render only labels for the control screen.
            // This might be inefficient if the control screen has been resized.
            // But usually the cached pages for presentation screen and control screen should not differ.
            last_cached++;
            cacheSize += ui->notes_label->updateCache(notes->getPage(last_cached))
                         + ui->current_slide_label->updateCache(presentation->getPage(last_cached));
        }
    }
}

void ControlScreen::receiveCache(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index)
{
    if (presentationScreen->getLabel()->cacheContains(index)) {
        qDebug() << "Page is already cached:" << index << ". Cache size" << cacheSize << "B";
        presentationScreen->getLabel()->updateCache(pres, index);
        ui->notes_label->updateCache(note, index);
        ui->current_slide_label->updateCache(small, index);
    }
    else {
        cacheSize += presentationScreen->getLabel()->updateCache(pres, index)
                     + ui->notes_label->updateCache(note, index)
                     + ui->current_slide_label->updateCache(small, index);
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

void ControlScreen::setCacheNumber(const int number)
{
    // Set maximum number of slides, which should be rendered to cache.
    // A negative number is interpreted as infinity.
    if (number < 0)
        maxCacheNumber = numberOfPages;
    else if (number==0) {
        ui->current_slide_label->setUseCache(false);
        ui->notes_label->setUseCache(false);
        presentationScreen->getLabel()->setUseCache(false);
        maxCacheNumber = 0;
    }
    else
        maxCacheNumber = number;
}

void ControlScreen::setCacheSize(const long size)
{
    // Set maximum memory used for cached pages (in bytes).
    // A negative number is interpreted as infinity.
    if (cacheSize==0) {
        ui->current_slide_label->setUseCache(false);
        ui->notes_label->setUseCache(false);
        presentationScreen->getLabel()->setUseCache(false);
    }
    maxCacheSize = size;
}

void ControlScreen::setTocLevel(const int level)
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
        emit sendNewPageNumber(pageNumber);
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

void ControlScreen::receivePageShiftReturn(int const shift)
{
    // Go to page shifted relative to the page shown on the presentation screen page.
    int pageNumber = presentationScreen->getPageNumber() + shift;
    renderPage(pageNumber);
    ui->label_timer->continueTimer();
    updateCache();
}

void ControlScreen::receiveCloseSignal()
{
    close();
}

void ControlScreen::keyPressEvent(QKeyEvent* event)
{
    // Key codes are given as key + modifiers.
    QMap<int, QList<int>>::iterator map_it = keymap->find(event->key() + event->modifiers());
    if (map_it==keymap->end())
        return;
    for (QList<int>::const_iterator action_it=map_it->cbegin(); action_it!=map_it->cend(); action_it++) {
        switch (*action_it) {
        case KeyAction::Next:
            currentPageNumber = presentationScreen->getPageNumber() + 1;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case KeyAction::Previous:
            currentPageNumber = presentationScreen->getPageNumber() - 1;
            if (currentPageNumber >= 0) {
                emit sendNewPageNumber(currentPageNumber);
                renderPage(currentPageNumber);
                showNotes();
                ui->label_timer->continueTimer();
                updateCache();
            }
            else
                currentPageNumber = 0;
            break;
        case KeyAction::NextCurrentScreen:
            renderPage(++currentPageNumber);
            showNotes();
            break;
        case KeyAction::PreviousCurrentScreen:
            if (currentPageNumber > 0)
                renderPage(--currentPageNumber);
            showNotes();
            break;
        case KeyAction::NextSkippingOverlays:
            currentPageNumber = notes->getNextSlideIndex(presentationScreen->getPageNumber());
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case KeyAction::PreviousSkippingOverlays:
            currentPageNumber = notes->getPreviousSlideEnd(presentationScreen->getPageNumber());
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case KeyAction::Update:
            currentPageNumber = presentationScreen->getPageNumber();
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case KeyAction::LastPage:
            currentPageNumber = numberOfPages - 1;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            updateCache();
            break;
        case KeyAction::FirstPage:
            currentPageNumber = 0;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            showNotes();
            updateCache();
            break;
        case KeyAction::UpdateCache:
            updateCache();
            break;
        case KeyAction::StartEmbeddedCurrentSlide:
            presentationScreen->getLabel()->startAllEmbeddedApplications(presentationScreen->getPageNumber());
            //ui->notes_label->startAllEmbeddedApplications(currentPageNumber);
            break;
        case KeyAction::StartAllEmbedded:
            startAllEmbeddedApplications();
            break;
        case KeyAction::GoToPage:
            showNotes();
            ui->text_current_slide->setFocus();
            break;
        case KeyAction::PlayMultimedia:
            {
                bool running = ui->notes_label->hasActiveMultimediaContent() || presentationScreen->getLabel()->hasActiveMultimediaContent();
                if (running)
                    emit pauseMultimedia();
                else
                    emit playMultimedia();
            }
            break;
        case KeyAction::ToggleCursor:
            emit togglePointerVisibilitySignal();
            break;
        case KeyAction::PauseTimer:
            ui->label_timer->pauseTimer();
            break;
        case KeyAction::ResetTimer:
            ui->label_timer->resetTimer();
            break;
        case KeyAction::ShowTOC:
            showToc();
            break;
        case KeyAction::HideTOC:
            hideToc();
            break;
        case KeyAction::ShowOverview:
            showOverview();
            break;
        case KeyAction::HideOverview:
            hideOverview();
            break;
        case KeyAction::Reload:
            reloadFiles();
            break;
        case KeyAction::SyncFromControlScreen:
            if (presentationScreen->getLabel()->pageNumber() != currentPageNumber)
                emit sendNewPageNumber(currentPageNumber);
            showNotes();
            updateCache();
            ui->label_timer->continueTimer();
            break;
        case KeyAction::SyncFromPresentationScreen:
            if (presentationScreen->getPageNumber() != currentPageNumber) {
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
        }
    }
    event->accept();
}

void ControlScreen::startAllEmbeddedApplications()
{
    // Start all embedded applications of the presentation on all pages.
    qDebug() << "Starting all embedded applications on all pages.";
    QList<Poppler::Page*> const pages = *presentation->getPages();
    PageLabel* label = presentationScreen->getLabel();
    for (QList<Poppler::Page*>::const_iterator page_it=pages.cbegin(); page_it!=pages.cend(); page_it++) {
        label->initEmbeddedApplications(*page_it);
        label->startAllEmbeddedApplications((*page_it)->index());
    }
}

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
    ui->notes_label->clearCache();
    ui->current_slide_label->clearCache();
    ui->next_slide_label->clearCache();
    overviewBox->setOutdated();
    // Render current page.
    ui->notes_label->renderPage(ui->notes_label->getPage(), false);
    ui->current_slide_label->renderPage(ui->current_slide_label->getPage(), false);
    ui->next_slide_label->renderPage(ui->next_slide_label->getPage(), false);
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
    newPalette.setColor(QPalette::Text, textColor);
    newPalette.setColor(QPalette::WindowText, textColor);
    setPalette(newPalette);
    newPalette.setColor(QPalette::Base, bgColor);
    ui->text_current_slide->setPalette(newPalette);
}

void ControlScreen::setPresentationColor(const QColor color)
{
    // set background color for the presentation screen.
    QPalette newPalette(presentationScreen->palette());
    newPalette.setColor(QPalette::Background, color);
    presentationScreen->setPalette(newPalette);
}

void ControlScreen::setScrollDelta(const int scrollDelta)
{
    this->scrollDelta = scrollDelta;
    presentationScreen->setScrollDelta(scrollDelta);
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
    // If a mouse wheel was used for scrolling:
    else {
        if (deltaAngle > 120)
            deltaPages = deltaAngle / 120;
        else if (deltaAngle > 0)
            deltaPages = 1;
        else if (deltaAngle < -120)
            deltaPages = deltaAngle / 120 + 1;
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

void ControlScreen::setEmbedFileList(const QStringList &files)
{
    // Set list of files, which should be be executed as embedded widgets if they are linked in the PDF.
    ui->notes_label->setEmbedFileList(files);
    presentationScreen->getLabel()->setEmbedFileList(files);
}

void ControlScreen::setPid2WidConverter(QString const &program)
{
    // Set an external program for converting process IDs to window IDs.
    QFileInfo fileinfo = QFileInfo(program);
    if (fileinfo.isFile() && fileinfo.isExecutable()) {
        ui->notes_label->setPid2Wid(program);
        presentationScreen->getLabel()->setPid2Wid(program);
    }
    else
        qCritical() << "Can't use program: not a file or not executable." << program;
}

void ControlScreen::setUrlSplitCharacter(QString const &splitCharacter)
{
    // Set a character, which splits links to files into file paths and arguments.
    ui->notes_label->setUrlSplitCharacter(splitCharacter);
    presentationScreen->getLabel()->setUrlSplitCharacter(splitCharacter);
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
    ui->notes_label->hide();
    tocBox->show();
    tocBox->setFocus();
}

void ControlScreen::hideToc()
{
    // Hide table of contents
    tocBox->hide();
    ui->notes_label->show();
    ui->notes_label->setFocus();
}

void ControlScreen::showNotes()
{
    // Equivalent to hideToc(); hideOverview();
    tocBox->hide();
    overviewBox->hide();
    ui->notes_label->show();
    ui->notes_label->setFocus();
}

void ControlScreen::showOverview()
{
    // Show overview on the control screen (above the notes label).
    hideToc();
    if (overviewBox->needsUpdate()) {
        cacheThread->requestInterruption();
        cacheTimer->stop();
        overviewBox->create(presentation, overviewColumns);
    }
    if (!this->isActiveWindow())
        this->activateWindow();
    ui->notes_label->hide();
    overviewBox->show();
    overviewBox->setFocus();
}

void ControlScreen::hideOverview()
{
    // Hide overview
    overviewBox->hide();
    ui->notes_label->show();
    ui->notes_label->setFocus();
}

bool ControlScreen::setRenderer(QStringList command)
{
    // Set a command for an external renderer.
    // This function also checks whether the command uses the arguments %file, %page, %width and %height

    if (command.size() == 1) {
        if (command[0]=="poppler") {
            cacheThread->setRenderer(Renderer::poppler);
            return true;
        }
        if (command[0]=="custom") {
            if (cacheThread->hasRenderCommand()) {
                cacheThread->setRenderer(Renderer::custom);
                return true;
            }
            qCritical() << "Ignored request to use undefined custom renderer";
        }
        else
            qCritical() << "Ignored request to use custom renderer without arguments";
        return false;
    }
    QString program = command[0];
    command.removeFirst();
    if (command.filter("%file").isEmpty()) {
        qCritical() << "Ignored request to use custom renderer without %file in arguments";
        return false;
    }
    if (command.filter("%page").isEmpty()) {
        qCritical() << "Ignored request to use custom renderer without %page in arguments";
        return false;
    }
    if (command.filter("%width").isEmpty())
        qWarning() << "Custom renderer does not use %width in arguments";
    if (command.filter("%height").isEmpty())
        qWarning() << "Custom renderer does not use %height in arguments";
    cacheThread->setCustomRenderer(program, presentation->getPath(), notes->getPath(), command);
    return true;
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
        ui->notes_label->clearAll();
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
        presentationScreen->renderPage(presentationScreen->getLabel()->pageNumber(), false);
        ui->current_slide_label->clearAll();
        ui->next_slide_label->clearAll();
        // Hide TOC and overview and set them outdated
        showNotes();
        tocBox->setOutdated();
        tocBox->createToc(presentation->getToc());
        overviewBox->setOutdated();
    }
    // If one of the two files has changed: Reset cache region and render pages on control screen.
    if (change) {
        qWarning() << "Reloading files is experimental!";
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

void ControlScreen::setKeyMap(QMap<int, QList<int>> *keymap)
{
    // Set the key bindings
    delete this->keymap;
    this->keymap = keymap;
    presentationScreen->setKeyMap(keymap);
}

void ControlScreen::setKeyMapItem(const int key, const int action)
{
    // Add an action to a key
    QMap<int, QList<int>>::iterator map_it = keymap->find(key);
    if (map_it==keymap->end())
        keymap->insert(key, {action});
    else
        map_it->append(action);
}
