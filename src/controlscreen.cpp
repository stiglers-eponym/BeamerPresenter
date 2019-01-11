/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "controlscreen.h"
#include "ui_controlscreen.h"

ControlScreen::ControlScreen(QString presentationPath, QString notesPath, QWidget * parent) :
    QMainWindow(parent),
    ui(new Ui::ControlScreen)
{
    ui->setupUi(this);

    // Load presentation pdf
    presentation = new PdfDoc( presentationPath );
    presentation->loadDocument();
    numberOfPages = presentation->popplerDoc->numPages();

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
        setWindowTitle("BeamerPresenter: " + notesPath);
    }

    // Set up the widgets
    ui->text_number_slides->setText( QString::fromStdString( std::to_string(numberOfPages) ) );
    ui->text_current_slide->setNumberOfPages(numberOfPages);
    ui->notes_label->setPresentationStatus(false);
    ui->current_slide_label->setShowMultimedia(false);
    ui->next_slide_label->setShowMultimedia(false);
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
    connect(presentationScreen, &PresentationScreen::sendUpdateCache, this, &ControlScreen::updateCache);
    connect(presentationScreen->getLabel(), &PageLabel::requestMultimediaSliders, this, &ControlScreen::addMultimediaSliders);

    // Signals sent back to PresentationScreen
    connect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(this, &ControlScreen::sendCloseSignal,   presentationScreen, &PresentationScreen::receiveCloseSignal);
    connect(this, &ControlScreen::sendUpdateCache,   presentationScreen, &PresentationScreen::updateCache);
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

    // Signals emitted by the page number editor
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn,  presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit,   this, &ControlScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit,    this, &ControlScreen::receivePageShiftEdit);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn,  this, &ControlScreen::receivePageShiftReturn);
    connect(ui->text_current_slide, &PageNumberEdit::sendEscape,           this, &ControlScreen::resetFocus);
}

ControlScreen::~ControlScreen()
{
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
    double screenRatio = double(height()) / width();
    QSize notesSize = notes->getPageSize(pageNumber);
    double notesSizeRatio = double(notesSize.height()) / notesSize.width();
    if (pagePart != 0)
        notesSizeRatio *= 2;
    double relativeNotesWidth = notesSizeRatio / screenRatio;
    if (relativeNotesWidth > 0.75)
        relativeNotesWidth = 0.75;
    ui->notes_label->setGeometry(0, 0, int(relativeNotesWidth*width()), height());
    ui->current_slide_label->setMaximumWidth( int( (1-relativeNotesWidth)*width() ) );
    ui->next_slide_label->setMaximumWidth( int( (1-relativeNotesWidth)*width() ) );
    ui->gridLayout->setColumnStretch(0, int( relativeNotesWidth*width() ));
    ui->gridLayout->setColumnStretch(1, int( (1-relativeNotesWidth)*width() ));
    updateGeometry();
}

void ControlScreen::focusPageNumberEdit()
{
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
        renderPage( presentationScreen->getPageNumber() );
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

void ControlScreen::renderPage( int const pageNumber )
{
    if (pageNumber < 0 || pageNumber >= numberOfPages)
        currentPageNumber = numberOfPages - 1;
    else
        currentPageNumber = pageNumber;
    recalcLayout(currentPageNumber);
    if (currentPageNumber < notes->popplerDoc->numPages())
        ui->notes_label->renderPage( notes->getPage(currentPageNumber) );
    if (currentPageNumber + 1 < presentation->popplerDoc->numPages()) {
        ui->current_slide_label->renderPage( presentation->getPage(currentPageNumber) );
        ui->current_slide_label->updateCache( presentation->getPage(currentPageNumber+1) );
        ui->next_slide_label->updateCache( ui->current_slide_label->getCache(), currentPageNumber+1 );
        ui->next_slide_label->renderPage( presentation->getPage(currentPageNumber+1) );
    }
    else {
        ui->current_slide_label->updateCache( presentation->getPage(currentPageNumber) );
        ui->current_slide_label->renderPage( presentation->getPage(currentPageNumber) );
        ui->next_slide_label->updateCache( ui->current_slide_label->getCache(), currentPageNumber );
        ui->next_slide_label->renderPage( presentation->getPage(currentPageNumber) );
    }
    ui->text_current_slide->setText( QString::fromStdString( std::to_string(currentPageNumber+1) ) );
}

void ControlScreen::updateCache()
{
    if (currentPageNumber + 1 < notes->popplerDoc->numPages()) {
        ui->notes_label->updateCache( notes->getPage(currentPageNumber+1) );
        if (currentPageNumber + 2 < presentation->popplerDoc->numPages())
            ui->next_slide_label->updateCache( presentation->getPage(currentPageNumber+2) );
    }
}

void ControlScreen::receiveNewPageNumber(int const pageNumber)
{
    renderPage( pageNumber );
}

void ControlScreen::receivePageShiftEdit(int const shift)
{
    renderPage( currentPageNumber + shift );
}

void ControlScreen::receivePageShiftReturn(int const shift)
{
    int pageNumber = presentationScreen->getPageNumber() + shift;
    renderPage( pageNumber );
    ui->label_timer->continueTimer();
}

void ControlScreen::receiveCloseSignal()
{
    close();
}

void ControlScreen::keyPressEvent( QKeyEvent * event )
{
    switch ( event->key() ) {
        case Qt::Key_Right:
        case Qt::Key_PageDown:
            emit sendNewPageNumber( ++currentPageNumber );
            renderPage( currentPageNumber );
            ui->label_timer->continueTimer();
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_Left:
        case Qt::Key_PageUp:
            if (currentPageNumber > 0) {
                emit sendNewPageNumber( --currentPageNumber );
                renderPage( currentPageNumber );
                ui->label_timer->continueTimer();
                emit sendUpdateCache();
                updateCache();
            }
            break;
        case Qt::Key_Down:
            currentPageNumber = notes->getNextSlideIndex(currentPageNumber);
            emit sendNewPageNumber( currentPageNumber );
            renderPage( currentPageNumber );
            ui->label_timer->continueTimer();
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_Up:
            currentPageNumber = notes->getPreviousSlideEnd(currentPageNumber);
            emit sendNewPageNumber( currentPageNumber );
            renderPage( currentPageNumber );
            ui->label_timer->continueTimer();
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_Space:
            emit sendNewPageNumber( currentPageNumber );
            renderPage( currentPageNumber );
            ui->label_timer->continueTimer();
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_End:
            currentPageNumber = numberOfPages - 1;
            emit sendNewPageNumber( currentPageNumber );
            renderPage( currentPageNumber );
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_Home:
            currentPageNumber = 0;
            emit sendNewPageNumber( currentPageNumber );
            renderPage( currentPageNumber );
            emit sendUpdateCache();
            updateCache();
            break;
        case Qt::Key_Q:
            emit sendCloseSignal();
            close();
            break;
        case Qt::Key_G:
            ui->text_current_slide->setText("");
            ui->text_current_slide->setFocus();
            break;
        case Qt::Key_P:
            ui->label_timer->pauseTimer();
            break;
        case Qt::Key_R:
            ui->label_timer->resetTimer();
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
        case Qt::Key_F:
        case Qt::Key_F11:
            if (this->windowState() == Qt::WindowFullScreen)
                showNormal();
            else
                showFullScreen();
            break;
    }
    event->accept();
}

void ControlScreen::resizeEvent(QResizeEvent *event)
{
    recalcLayout(currentPageNumber);
    ui->notes_label->clearCache();
    ui->current_slide_label->clearCache();
    ui->next_slide_label->clearCache();
    ui->notes_label->renderPage( ui->notes_label->getPage() );
    ui->current_slide_label->renderPage( ui->current_slide_label->getPage() );
    ui->next_slide_label->renderPage( ui->next_slide_label->getPage() );
}
