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
    videoPlayers = QList<QMediaPlayer*>();
    videoPlaylists = QList<QMediaPlaylist*>();
    videoWidgets = QList<QVideoWidget*>();
    videoPositions = QList<QRect*>();
}

PageLabel::~PageLabel()
{
    qDeleteAll(links);
    links.clear();
    qDeleteAll(linkPositions);
    linkPositions.clear();
    qDeleteAll(videoPositions);
    videoPositions.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    qDeleteAll(videoPlaylists);
    videoPlaylists.clear();
    qDeleteAll(videoPlayers);
    videoPlayers.clear();
    page = nullptr;
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
    qDeleteAll(videoPlaylists);
    videoPlaylists.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    qDeleteAll(videoPlayers);
    videoPlayers.clear();

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
    if ( duration > 0)
        QTimer::singleShot(int(1000*duration), this, &PageLabel::timeoutSignal);
    Poppler::PageTransition* transition = page->transition();
    if (transition->type() != Poppler::PageTransition::Replace)
        std::cout << "Unsupported page transition of type " << transition->type() << std::endl;
    //if (page->annotations().size() != 0)
    //    std::cout << "This page contains annotations, which are not supported." << std::endl;

    // Show videos. This part is work in progress.
    // TODO: This is probably inefficient.
    if (showVideos) {
        QSet<Poppler::Annotation::SubType> videoType = QSet<Poppler::Annotation::SubType>();
        videoType.insert(Poppler::Annotation::AMovie);
        QList<Poppler::Annotation*> videos = page->annotations(videoType);
        for (int i=0; i<videos.size(); i++) {
            Poppler::MovieAnnotation* annotation = (Poppler::MovieAnnotation*) videos.at(i);
            Poppler::MovieObject * movie = annotation->movie();
            QRectF relative = annotation->boundary();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            movie->playMode();
            videoPositions.append(absolute);
            QMediaPlayer * player = new QMediaPlayer(this);
            videoPlayers.append(player);
            QMediaPlaylist * playlist = new QMediaPlaylist(player);
            videoPlaylists.append(playlist);
            QVideoWidget * video = new QVideoWidget(this);
            videoWidgets.append(video);
            //QPalette palette = QPalette();
            //palette.setColor(QPalette::Background, Qt::white);
            //video->setPalette(palette);
            //video->setAutoFillBackground(true);
            player->setPlaylist(playlist);
            player->setVideoOutput(video);
            video->setGeometry(*absolute);
            video->setMouseTracking(true); // TODO: It should be possible to do this more efficiently

            QUrl url = QUrl(movie->url());
            if (!url.isValid())
                QUrl url = QUrl::fromLocalFile(movie->url());
            if (url.isRelative())
                url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
            playlist->addMedia( url );
            // TODO: Check if poster image is shown correctly, otherwise show it with
            // if (movie->showPosterImage())
            video->show();
            playlist->next();
            switch (movie->playMode()) {
                case Poppler::MovieObject::PlayOpen:
                    // TODO: PlayOpen leaves controls open (but they are not implemented yet).
                case Poppler::MovieObject::PlayOnce:
                    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
                break;
                case Poppler::MovieObject::PlayPalindrome:
                    std::cout << "This video should be played in palindrome mode, which is not supported.\n"
                              << "The video will be played in loop mode instead." << std::endl;
                    // TODO: implement backward play and palindrome mode
                case Poppler::MovieObject::PlayRepeat:
                    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
                break;
            }
            player->play();
            if (movie->showControls() || !isPresentation) {
                // TODO: video control bar
            }
        }
        qDeleteAll(videos);
        videos.clear();
    }
}

void PageLabel::setPresentationStatus(bool const isPresentation)
{
    this->isPresentation = isPresentation;
}

void PageLabel::setShowVideos(const bool showVideos)
{
    this->showVideos = showVideos;
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
                        std::cout << "Unsupported link of type movie" << std::endl;
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
        for (int i=0; i<videoPositions.size(); i++) {
            if ( videoPositions.at(i)->contains(event->pos()) ) {
                if ( videoPlayers.at(i)->state() == QMediaPlayer::PlayingState )
                    videoPlayers.at(i)->pause();
                else
                    videoPlayers.at(i)->play();
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
        this->setCursor( Qt::BlankCursor );
    }
    else {
        pointer_visible = true;
        setMouseTracking(true);
        this->setCursor( Qt::ArrowCursor );
    }
}

void PageLabel::mouseMoveEvent(QMouseEvent * event)
{
    if (!pointer_visible)
        return;
    bool is_arrow_pointer = this->cursor() == Qt::ArrowCursor;
    Q_FOREACH(QRect* link_rect, linkPositions) {
        if (link_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                this->setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    Q_FOREACH(QRect* video_rect, videoPositions) {
        if (video_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                this->setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    if (!is_arrow_pointer)
        this->setCursor(Qt::ArrowCursor);
}
