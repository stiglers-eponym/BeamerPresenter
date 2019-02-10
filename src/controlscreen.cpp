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
            exit(1);
        }
        QFileInfo checkPresentation(presentationPath);
        if (!checkPresentation.exists() || (!checkPresentation.isFile() && !checkPresentation.isSymLink()) ) {
            qCritical() << "Not a file: " << presentationPath;
            close();
            exit(1);
        }
        if (!notesPath.isEmpty()) {
            QFileInfo checkNotes(notesPath);
            if (!checkNotes.exists() || (!checkNotes.isFile() && !checkNotes.isSymLink()) ) {
                qWarning() << "WARNING: Ignoring invalid notes files: " << notesPath;
                notesPath = "";
            }
        }
    }
    ui->setupUi(this);

    // Load presentation pdf
    presentation = new PdfDoc( presentationPath );
    presentation->loadDocument();
    if (presentation->getDoc() == nullptr){
        qCritical() << "File could not be opened as PDF: " << presentationPath;
        close();
        exit(1);
    }
    numberOfPages = presentation->getDoc()->numPages();
    maxCacheNumber = numberOfPages;
    last_delete = numberOfPages-1;

    // Set up presentation screen
    presentationScreen = new PresentationScreen( presentation );
    presentationScreen->setWindowTitle("BeamerPresenter: " + presentationPath);

    // Load notes pdf
    if (notesPath.isEmpty()) {
        notes = presentation;
        setWindowTitle("BeamerPresenter: " + presentationPath);
    }
    else {
        notes = new PdfDoc( notesPath );
        notes->loadDocument();
        if (notes->getDoc() == nullptr) {
            qWarning() << "File could not be opened as PDF: " << notesPath;
            notes = presentation;
            setWindowTitle("BeamerPresenter: " + presentationPath);
        }
        else
            setWindowTitle("BeamerPresenter: " + notesPath);
    }

    // Set up the widgets
    ui->text_number_slides->setText( QString::fromStdString( std::to_string(numberOfPages) ) );
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

    ui->label_timer->setTimerWidget( ui->edit_timer );
    // Signals emitted by the timer
    connect(ui->label_timer, &Timer::sendAlert,   this, &ControlScreen::receiveTimerAlert);
    connect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    connect(ui->label_timer, &Timer::sendEscape,  this, &ControlScreen::resetFocus);
    // Signals sent back to the timer
    connect(this, &ControlScreen::sendTimerString,     ui->label_timer, &Timer::receiveTimerString);
    connect(this, &ControlScreen::sendTimeoutInterval, ui->label_timer, &Timer::receiveTimeoutInterval);

    // Signals sent to the page labels
    connect(this, &ControlScreen::sendAutostartDelay, ui->notes_label, &PageLabel::setAutostartDelay);
    connect(this, &ControlScreen::sendAutostartDelay, presentationScreen->getLabel(), &PageLabel::setAutostartDelay);
    connect(this, &ControlScreen::playMultimedia,     ui->notes_label, &PageLabel::startAllMultimedia);
    connect(this, &ControlScreen::playMultimedia,     presentationScreen->getLabel(), &PageLabel::startAllMultimedia);
    connect(this, &ControlScreen::pauseMultimedia,    ui->notes_label, &PageLabel::pauseAllMultimedia);
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
}

ControlScreen::~ControlScreen()
{
    cacheThread->requestInterruption();
    cacheTimer->stop();
    cacheTimer->disconnect();
    delete cacheTimer;
    if (cacheThread->isRunning())
        cacheThread->terminate();
    cacheThread->deleteLater();
    ui->notes_label->disconnect();
    ui->current_slide_label->disconnect();
    ui->next_slide_label->disconnect();
    ui->label_timer->disconnect();
    ui->text_current_slide->disconnect();
    presentationScreen->getLabel()->disconnect();
    presentationScreen->disconnect();
    if (notes != presentation)
        delete notes;
    delete presentation;
    delete presentationScreen;
    disconnect();
    delete ui;
}

void ControlScreen::setPagePart(int const pagePart)
{
    this->pagePart = pagePart;
    ui->notes_label->setPagePart(-pagePart);
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
    if (pagePart != 0)
        notesSizeRatio *= 2;
    double relativeNotesWidth = notesSizeRatio / screenRatio;
    if (relativeNotesWidth > 0.75)
        relativeNotesWidth = 0.75;
    int sideWidth = int((1-relativeNotesWidth)*width());

    // Set layout
    ui->notes_label->setGeometry(0, 0, width()-sideWidth, height());
    ui->current_slide_label->setMaximumWidth(sideWidth);
    ui->next_slide_label->setMaximumWidth(sideWidth);
    ui->gridLayout->setColumnStretch(0, width()-sideWidth);
    ui->gridLayout->setColumnStretch(1, sideWidth);
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
    QList<MediaSlider*> sliderList = QList<MediaSlider*>();
    for (int i=0; i<n; i++) {
        MediaSlider * slider = new MediaSlider(this);
        ui->overviewLayout->addWidget(slider);
        sliderList.append(slider);
        connect(slider, &MediaSlider::sendEscapeEvent, this, &ControlScreen::resetFocus);
        connect(slider, &MediaSlider::sendKeyEvent, this, &ControlScreen::keyPressEvent);
    }
    presentationScreen->getLabel()->setMultimediaSliders(sliderList);
}

void ControlScreen::resetFocus()
{
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
    if (pageNumber < 0 || pageNumber >= numberOfPages)
        currentPageNumber = numberOfPages - 1;
    else
        currentPageNumber = pageNumber;
    recalcLayout(currentPageNumber);
    if (currentPageNumber < notes->getDoc()->numPages())
        ui->notes_label->renderPage( notes->getPage(currentPageNumber), false );
    if (currentPageNumber + 1 < presentation->getDoc()->numPages()) {
        ui->current_slide_label->renderPage(presentation->getPage(currentPageNumber), false);
        if (maxCacheSize!=0 && maxCacheNumber!=0) {
            ui->current_slide_label->updateCache(presentation->getPage(currentPageNumber+1));
            QPixmap* pixmap = ui->current_slide_label->getCache(currentPageNumber+1);
            ui->next_slide_label->renderPage(presentation->getPage(currentPageNumber+1), false, pixmap);
            delete pixmap;
        }
        else {
            ui->next_slide_label->renderPage(presentation->getPage(currentPageNumber+1));
        }
    }
    else {
        Poppler::Page* page = presentation->getPage(currentPageNumber);
        QPixmap* pixmap = ui->current_slide_label->getCache(currentPageNumber);
        if (pixmap->isNull()) {
            ui->current_slide_label->renderPage(page);
            delete pixmap;
            pixmap = ui->current_slide_label->getCache(currentPageNumber);
        }
        else {
            ui->current_slide_label->renderPage(page, false, pixmap);
        }
        ui->next_slide_label->renderPage(page, false, pixmap);
        delete pixmap;
    }
    ui->text_current_slide->setText(QString::fromStdString(std::to_string(currentPageNumber+1)));
}

void ControlScreen::updateCache()
{
    if (maxCacheSize == 0 || maxCacheNumber == 0)
        return;
    cacheTimer->stop();
    // Number of currently cached slides
    cacheNumber = presentationScreen->getLabel()->getCacheNumber();
    if (cacheNumber == numberOfPages)
        return;
    // Size of currently cached slides (set to -infinity if it shoule be ignored)
    if (maxCacheSize > 0)
        cacheSize = presentationScreen->getLabel()->getCacheSize()
             + ui->notes_label->getCacheSize()
             + ui->current_slide_label->getCacheSize();
    else
        cacheSize = -4294967296;

    // Current page:
    if (first_cached > currentPageNumber || last_cached < currentPageNumber) {
        // we are outside the simply connected cache region: reset cache numbers
        cacheThread->requestInterruption();
        first_cached = currentPageNumber;
        last_cached = currentPageNumber-1;
        first_delete = 0;
        last_delete = numberOfPages-1;
        qDebug() << "Reset cache region" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
    }
    else {
        last_delete = last_delete > currentPageNumber + cacheNumber ? last_delete : currentPageNumber + cacheNumber;
        last_delete = last_delete >= numberOfPages ? numberOfPages-1 : last_delete;
        first_delete = first_delete > currentPageNumber - cacheNumber/2 ? currentPageNumber - cacheNumber/2 : first_delete;
        first_delete = first_delete < 0 ? 0 : first_delete;
    }
    if (last_cached < numberOfPages-1 || first_cached > 0)
        cacheTimer->start();
}

void ControlScreen::updateCacheStep()
{
    // TODO: improve this, make it more deterministic, avoid caching pages which will directly be freed again
    if (last_cached > last_delete || first_cached < first_delete || first_cached > currentPageNumber || last_cached < currentPageNumber-1 || cacheThread->isRunning()) {
        cacheTimer->stop();
        qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete << cacheThread->isRunning();
        return;
    }
    // Free space if necessary
    long int delta;
    while (cacheSize > maxCacheSize || cacheNumber > maxCacheNumber) {
        // Approximately 1/3 of the cached slides should be previous slides
        if (last_delete > 3*currentPageNumber - 2*first_delete) {
            delta = presentationScreen->getLabel()->clearCachePage(last_delete);
            if (delta != 0) {
                cacheNumber--;
                cacheSize -= delta;
                qDebug() << "Freed last page" << last_delete << ". Cache size" << cacheSize << "B";
            }
            cacheSize -= ui->notes_label->clearCachePage(last_delete)
                    + ui->current_slide_label->clearCachePage(last_delete);
            last_delete--;
            last_cached = last_delete > last_cached ? last_cached : last_delete;
        }
        else {
            delta = presentationScreen->getLabel()->clearCachePage(first_delete);
            if (delta != 0) {
                cacheNumber--;
                cacheSize -= delta;
                qDebug() << "Freed first page" << first_delete << ". Cache size" << cacheSize << "B";
            }
            cacheSize -= ui->notes_label->clearCachePage(first_delete)
                    + ui->current_slide_label->clearCachePage(first_delete);
            first_delete++;
            first_cached = first_cached > first_delete ? first_cached : first_delete;
        }
        if (last_cached > last_delete || first_cached < first_delete) {
            cacheTimer->stop();
            qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
            return;
        }
    }
    if (last_cached == numberOfPages-1 && first_cached > first_delete && 2*maxCacheNumber > 3*cacheNumber && 2*maxCacheSize > 3*cacheSize) {
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
    }
    else if ( (last_cached == last_delete || 3*(last_cached-currentPageNumber) >= 2*maxCacheNumber)
            && (cacheNumber >= maxCacheNumber || (maxCacheSize-cacheSize)*cacheNumber < cacheSize) ) {
        cacheTimer->stop();
        qDebug() << "Stopped cache timer" << first_delete << first_cached << currentPageNumber << last_cached << last_delete;
        return;
    }
    // Cache the page last_cached+1
    if (!presentationScreen->getLabel()->cacheContains(last_cached+1) && last_cached+1 < numberOfPages) {
        cacheTimer->stop();
        cacheThread->setPages(presentation->getPage(last_cached+1), notes->getPage(last_cached+1));
        cacheThread->start();
    }
    else {
        last_cached++;
        cacheSize += ui->notes_label->updateCache(notes->getPage(last_cached))
                     + ui->current_slide_label->updateCache(presentation->getPage(last_cached));
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
        if (cacheNumber==numberOfPages)
            qInfo() << "All slides are rendered to cache. Cache size:" << cacheSize << "B";
        qDebug() << "Cached page" << index << ". Cache size" << cacheSize << "B";
    }
    if (index == last_cached+1)
        last_cached++;
    else if (index == first_cached-1)
        first_cached--;
    cacheThread->wait(100);
    cacheTimer->start();
}

void ControlScreen::setCacheNumber(const int number)
{
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
    if (cacheSize==0) {
        ui->current_slide_label->setUseCache(false);
        ui->notes_label->setUseCache(false);
        presentationScreen->getLabel()->setUseCache(false);
    }
    maxCacheSize = size;
}

void ControlScreen::receiveNewPageNumber(int const pageNumber)
{
    renderPage(pageNumber);
    updateCache();
}

void ControlScreen::receivePageShiftEdit(int const shift)
{
    if (currentPageNumber + shift >= 0) {
        renderPage(currentPageNumber + shift);
        updateCache();
    }
}

void ControlScreen::receivePreviousSlideEnd()
{
    if (currentPageNumber >= 0) {
        renderPage(notes->getPreviousSlideEnd(currentPageNumber));
        updateCache();
    }
}

void ControlScreen::receiveNextSlideStart()
{
    if (currentPageNumber < numberOfPages - 1) {
        renderPage(notes->getNextSlideIndex(currentPageNumber));
        updateCache();
    }
}

void ControlScreen::receivePageShiftReturn(int const shift)
{
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
    switch (event->key()) {
        case Qt::Key_Right:
        case Qt::Key_PageDown:
            currentPageNumber = presentationScreen->getPageNumber() + 1;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case Qt::Key_Left:
        case Qt::Key_PageUp:
            currentPageNumber = presentationScreen->getPageNumber() - 1;
            if (currentPageNumber >= 0) {
                emit sendNewPageNumber(currentPageNumber);
                renderPage(currentPageNumber);
                ui->label_timer->continueTimer();
                updateCache();
            }
            else
                currentPageNumber = 0;
            break;
        case Qt::Key_Down:
            currentPageNumber = notes->getNextSlideIndex(presentationScreen->getPageNumber());
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case Qt::Key_Up:
            currentPageNumber = notes->getPreviousSlideEnd(presentationScreen->getPageNumber());
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case Qt::Key_Space:
            currentPageNumber = presentationScreen->getPageNumber();
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            ui->label_timer->continueTimer();
            updateCache();
            break;
        case Qt::Key_C:
            updateCache();
            break;
        case Qt::Key_End:
            currentPageNumber = numberOfPages - 1;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            updateCache();
            break;
        case Qt::Key_Home:
            currentPageNumber = 0;
            emit sendNewPageNumber(currentPageNumber);
            renderPage(currentPageNumber);
            updateCache();
            break;
        case Qt::Key_Q:
            emit sendCloseSignal();
            close();
            break;
        case Qt::Key_G:
            ui->text_current_slide->setFocus();
            break;
        case Qt::Key_P:
            ui->label_timer->pauseTimer();
            break;
        case Qt::Key_R:
            ui->label_timer->resetTimer();
            break;
        case Qt::Key_O:
            emit togglePointerVisibilitySignal();
            break;
        case Qt::Key_M:
            {
                bool running = ui->notes_label->hasActiveMultimediaContent() || presentationScreen->getLabel()->hasActiveMultimediaContent();
                if (running)
                    emit pauseMultimedia();
                else
                    emit playMultimedia();
            }
            break;
        case Qt::Key_Return:
            if (presentationScreen->getLabel()->pageNumber() != currentPageNumber)
                emit sendNewPageNumber(currentPageNumber);
            updateCache();
            ui->label_timer->continueTimer();
            break;
        case Qt::Key_F:
        case Qt::Key_F11:
            if (this->windowState() == Qt::WindowFullScreen)
                showNormal();
            else
                showFullScreen();
            break;
        case Qt::Key_E:
            presentationScreen->getLabel()->startAllEmbeddedApplications();
            ui->notes_label->startAllEmbeddedApplications();
            break;
        case Qt::Key_Escape:
            if (presentationScreen->getPageNumber() != currentPageNumber) {
                currentPageNumber = presentationScreen->getPageNumber();
                renderPage(currentPageNumber);
                updateCache();
            }
    }
    event->accept();
}

void ControlScreen::resizeEvent(QResizeEvent* event)
{
    cacheThread->requestInterruption();
    recalcLayout(currentPageNumber);
    last_cached = presentationScreen->getPageNumber();
    first_cached = last_cached;
    first_delete = 0;
    last_delete = numberOfPages-1;
    ui->notes_label->clearCache();
    ui->current_slide_label->clearCache();
    ui->next_slide_label->clearCache();
    ui->notes_label->renderPage(ui->notes_label->getPage(), false);
    ui->current_slide_label->renderPage(ui->current_slide_label->getPage(), false);
    ui->next_slide_label->renderPage(ui->next_slide_label->getPage(), false);
}

void ControlScreen::clearPresentationCache()
{
    cacheThread->requestInterruption();
    last_cached = presentationScreen->getPageNumber();
    first_cached = last_cached;
    first_delete = 0;
    last_delete = numberOfPages-1;
}

void ControlScreen::setColor(const QColor bgColor, const QColor textColor)
{
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
    QPalette newPalette(presentationScreen->palette());
    newPalette.setColor(QPalette::Background, color);
    presentationScreen->setPalette(newPalette);
}

void ControlScreen::wheelEvent(QWheelEvent* event)
{
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
    ui->notes_label->setEmbedFileList(files);
    presentationScreen->getLabel()->setEmbedFileList(files);
}

void ControlScreen::setPid2WidConverter(QString const &program)
{
    QFileInfo fileinfo = QFileInfo(program);
    if (fileinfo.isFile() && fileinfo.isExecutable()) {
        ui->notes_label->setPid2Wid(program);
        presentationScreen->getLabel()->setPid2Wid(program);
    }
    else
        qWarning() << "Can't use program: not a file or not executable." << program;
}

void ControlScreen::setUrlSplitCharacter(QString const &splitCharacter)
{
    ui->notes_label->setUrlSplitCharacter(splitCharacter);
    presentationScreen->getLabel()->setUrlSplitCharacter(splitCharacter);
}
