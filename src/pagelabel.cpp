/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "pagelabel.h"

PageLabel::PageLabel(Poppler::Page* page, QWidget* parent) : QLabel(parent)
{
    renderPage(page);
}

PageLabel::PageLabel(QWidget* parent) : QLabel(parent)
{
    page = nullptr;
    links = QList<Poppler::Link*>();
    linkPositions = QList<QRect*>();
    videoWidgets = QList<VideoWidget*>();
    videoPositions = QList<QRect*>();
    soundPositions = QList<QRect*>();
    soundPlayers = QList<QMediaPlayer*>();
}

PageLabel::~PageLabel()
{
    delete timer;
    qDeleteAll(links);
    links.clear();
    qDeleteAll(linkPositions);
    linkPositions.clear();
    qDeleteAll(videoPositions);
    videoPositions.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    qDeleteAll(soundPositions);
    soundPositions.clear();
    qDeleteAll(soundPlayers);
    soundPlayers.clear();
    page = nullptr;
}

void PageLabel::setAutostartDelay(double const delay)
{
    autostartDelay = delay;
}

int PageLabel::pageNumber()
{
    return page->index();
}

void PageLabel::renderPage(Poppler::Page* page)
{
    qDeleteAll(links);
    links.clear();
    qDeleteAll(linkPositions);
    linkPositions.clear();
    qDeleteAll(videoPositions);
    videoPositions.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    qDeleteAll(soundPositions);
    soundPositions.clear();
    qDeleteAll(soundPlayers);
    soundPlayers.clear();

    this->page = page;
    QSize pageSize = page->pageSize();
    int shift_x=0, shift_y=0;
    double resolution;
    if ( width() * pageSize.height() > height() * pageSize.width() ) {
        // the width of the label is larger than required
        resolution = double( height() ) / pageSize.height();
        shift_x = int( width()/2 - resolution/2 * pageSize.width() );
    }
    else {
        // the height of the label is larger than required
        resolution = double( width() ) / pageSize.width();
        shift_y = int( height()/2 - resolution/2 * pageSize.height() );
    }
    double scale_x=resolution*pageSize.width(), scale_y=resolution*pageSize.height();
    setPixmap( QPixmap::fromImage( page->renderToImage( 72*resolution, 72*resolution ) ) );

    // Collect link areas in pixels
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        QRect * absolute = new QRect(
                    shift_x+int(relative.x()*scale_x),
                    shift_y+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                );
        linkPositions.append( absolute );
    }

    duration = page->duration();
    if ( duration > 0.01)
        QTimer::singleShot(int(1000*duration), this, &PageLabel::timeoutSignal);
    else if ( duration > -0.01) {
        update();
        QTimer::singleShot(int(20), this, &PageLabel::timeoutSignal);
    }
    Poppler::PageTransition* transition = page->transition();
    if (transition->type() != Poppler::PageTransition::Replace)
        std::cout << "Unsupported page transition of type " << transition->type() << std::endl;

    // Show videos. This part is work in progress.
    // TODO: This is probably inefficient.
    if (showMultimedia) {
        // Video
        QSet<Poppler::Annotation::SubType> videoType = QSet<Poppler::Annotation::SubType>();
        videoType.insert(Poppler::Annotation::AMovie);
        QList<Poppler::Annotation*> videos = page->annotations(videoType);
        for (int i=0; i<videos.size(); i++) {
            Poppler::MovieAnnotation * annotation = (Poppler::MovieAnnotation*) videos.at(i);
            QRectF relative = annotation->boundary();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            videoPositions.append(absolute);

            VideoWidget * video = new VideoWidget(annotation, this);
            videoWidgets.append(video);
        }
        videos.clear();

        // Audio (Untested!)
        QSet<Poppler::Annotation::SubType> soundType = QSet<Poppler::Annotation::SubType>();
        soundType.insert(Poppler::Annotation::ASound);
        QList<Poppler::Annotation*> sounds = page->annotations(soundType);
        for (int i=0; i<sounds.size(); i++) {
            std::cout << "WARNING: Support for sound is untested!" << std::endl;
            Poppler::SoundAnnotation* annotation = (Poppler::SoundAnnotation*) sounds.at(i);
            QRectF relative = annotation->boundary();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            soundPositions.append(absolute);

            Poppler::SoundObject * sound = annotation->sound();
            QMediaPlayer * player = new QMediaPlayer(this);
            QUrl url = QUrl(sound->url());
            if (!url.isValid())
                QUrl url = QUrl::fromLocalFile(sound->url());
            if (url.isRelative())
                url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
            player->setMedia( url );
            soundPlayers.append(player);
        }
        qDeleteAll(sounds);
        sounds.clear();

        // Autostart
        if (autostartDelay > 0.1) {
            // autostart with delay
            delete timer;
            timer = new QTimer();
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, this, &PageLabel::startAllMultimedia );
            timer->start(int(autostartDelay*1000));
        }
        else if (autostartDelay > -0.1) {
            // autostart without delay
            startAllMultimedia();
        }
    }
}

void PageLabel::startAllMultimedia()
{
    for (int i=0; i<videoWidgets.size(); i++) {
        videoWidgets.at(i)->setGeometry(*videoPositions.at(i));
        videoWidgets.at(i)->show();
        videoWidgets.at(i)->play();
    }
    Q_FOREACH(QMediaPlayer * sound, soundPlayers)
        sound->play();
}

void PageLabel::pauseAllMultimedia()
{
    Q_FOREACH(VideoWidget * video, videoWidgets)
        video->pause();
    Q_FOREACH(QMediaPlayer * sound, soundPlayers)
        sound->pause();
}

bool PageLabel::hasActiveMultimediaContent() const
{
    Q_FOREACH(VideoWidget * video, videoWidgets) {
        if (video->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer * sound, soundPlayers) {
        if (sound->state() == QMediaPlayer::PlayingState)
            return true;
    }
    return false;
}

void PageLabel::setPresentationStatus(bool const isPresentation)
{
    this->isPresentation = isPresentation;
}

void PageLabel::setShowMultimedia(const bool showMultimedia)
{
    this->showMultimedia = showMultimedia;
}

double PageLabel::getDuration() const
{
    return duration;
}

void PageLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            if ( linkPositions.at(i)->contains(event->pos()) ) {
                switch ( links.at(i)->linkType() )
                {
                    case Poppler::Link::Goto:
                        emit sendNewPageNumber( ((Poppler::LinkGoto*) links.at(i))->destination().pageNumber() - 1 );
                    break;
                    case Poppler::Link::Execute:
                        std::cout << "Unsupported link of type execute" << std::endl;
                    break;
                    case Poppler::Link::Browse:
                        QDesktopServices::openUrl( QUrl( ((Poppler::LinkBrowse*) links.at(i))->url(), QUrl::TolerantMode ) );
                    break;
                    case Poppler::Link::Action:
                        {
                            Poppler::LinkAction* actionLink = (Poppler::LinkAction*) links.at(i);
                            Poppler::LinkAction::ActionType action = actionLink->actionType();
                            std::cout << "Unsupported link of type action: ActionType = " << action << std::endl;
                        }
                    break;
                    case Poppler::Link::Sound:
                        std::cout << "Unsupported link of type sound" << std::endl;
                    break;
                    case Poppler::Link::Movie:
                        {
                            std::cout << "Unsupported link of type video." << std::endl;
                            Poppler::LinkMovie* link = (Poppler::LinkMovie*) links.at(i);
                            Q_FOREACH(VideoWidget * video, videoWidgets) {
                                if (link->isReferencedAnnotation(video->getAnnotation()))
                                    video->play();
                            }
                        }
                    break;
                    case Poppler::Link::Rendition:
                        std::cout << "Unsupported link of type rendition" << std::endl;
                    break;
                    case Poppler::Link::JavaScript:
                        std::cout << "Unsupported link of type JavaScript" << std::endl;
                    break;
                    case Poppler::Link::OCGState:
                        if ( linkPositions.at(i)->contains(event->pos()) )
                            std::cout << "Unsupported link of type OCGState" << std::endl;
                    break;
                    case Poppler::Link::Hide:
                        std::cout << "Unsupported link of type hide" << std::endl;
                    break;
                    case Poppler::Link::None:
                        std::cout << "Unsupported link of type none" << std::endl;
                    break;
                }
            }
        }
        for (int i=0; i<soundPositions.size(); i++) {
            if (soundPositions.at(i)->contains(event->pos())) {
                if (soundPlayers.at(i)->state() == QMediaPlayer::PlayingState)
                    soundPlayers.at(i)->pause();
                else
                    soundPlayers.at(i)->play();
            }
        }
    }
    event->accept();
}

void PageLabel::togglePointerVisibility()
{
    if ( pointer_visible ) {
        pointer_visible = false;
        setMouseTracking(false);
        setCursor( Qt::BlankCursor );
    }
    else {
        pointer_visible = true;
        setMouseTracking(true);
        setCursor( Qt::ArrowCursor );
    }
}

void PageLabel::mouseMoveEvent(QMouseEvent * event)
{
    if (!pointer_visible)
        return;
    bool is_arrow_pointer = cursor() == Qt::ArrowCursor;
    Q_FOREACH(QRect* link_rect, linkPositions) {
        if (link_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    Q_FOREACH(QRect* sound_rect, soundPositions) {
        if (sound_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    if (!is_arrow_pointer)
        setCursor(Qt::ArrowCursor);
    event->accept();
}
