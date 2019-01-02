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
}

PageLabel::~PageLabel()
{
    links.clear();
    linkPositions.clear();
}

int PageLabel::pageNumber()
{
    return page->index();
}

void PageLabel::renderPage(Poppler::Page* page)
{
    this->page = page;
    links.clear();
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
    linkPositions.clear();
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
    QSet<Poppler::Annotation::SubType> movieType = QSet<Poppler::Annotation::SubType>();
    movieType.insert(Poppler::Annotation::AMovie);
    QList<Poppler::Annotation*> movies = page->annotations(movieType);
    videoWidgets.clear();
    moviePositions.clear();
    for (int i=0; i<movies.size(); i++) {
        Poppler::MovieAnnotation* annotation = (Poppler::MovieAnnotation*) movies.at(i);
        QRectF relative = annotation->boundary();
        QRect * absolute = new QRect(
                    shift_x+int(relative.x()*scale_x),
                    shift_y+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                );
        if ( player == nullptr )
            player = new QMediaPlayer(this);
        if ( playlist == nullptr )
            playlist = new QMediaPlaylist(player);
        player->setPlaylist(playlist);
        // TODO: fix paths, allow for relative paths
        playlist->addMedia( QUrl::fromLocalFile(annotation->movie()->url()) );
        //if (annotation->movie()->showPosterImage())
        //    TODO: show poster image
        // TODO: controls, interaction
        QVideoWidget * video = new QVideoWidget(this);
        videoWidgets.append(video);
        player->setVideoOutput(video);
        video->setGeometry(*absolute);
        video->show();
        playlist->next();
        player->play();
    }
}

double PageLabel::getDuration() const
{
    return duration;
}

void PageLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            switch ( links.at(i)->linkType() )
            {
                case Poppler::Link::Goto:
                    if ( linkPositions.at(i)->contains(event->pos()) ) {
                        emit sendNewPageNumber( ((Poppler::LinkGoto*) links.at(i))->destination().pageNumber() - 1 );
                    }
                break;
                case Poppler::Link::Execute:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type execute" << std::endl;
                break;
                case Poppler::Link::Browse:
                    if ( linkPositions.at(i)->contains(event->pos()) ) {
                        QDesktopServices::openUrl( QUrl( ((Poppler::LinkBrowse*) links.at(i))->url(), QUrl::TolerantMode ) );
                    }
                break;
                case Poppler::Link::Action:
                    if ( linkPositions.at(i)->contains(event->pos()) ) {
                        Poppler::LinkAction* actionLink = (Poppler::LinkAction*) links.at(i);
                        Poppler::LinkAction::ActionType action = actionLink->actionType();
                        std::cout << "Unsupported link of type action: ActionType = " << action << std::endl;
                    }
                break;
                case Poppler::Link::Sound:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type sound" << std::endl;
                break;
                case Poppler::Link::Movie:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type movie" << std::endl;
                break;
                case Poppler::Link::Rendition:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type rendition" << std::endl;
                break;
                case Poppler::Link::JavaScript:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type JavaScript" << std::endl;
                break;
                case Poppler::Link::OCGState:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type OCGState" << std::endl;
                break;
                case Poppler::Link::Hide:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type hide" << std::endl;
                break;
                case Poppler::Link::None:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type none" << std::endl;
                break;
            }
        }
    }
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
    if (!is_arrow_pointer)
        this->setCursor(Qt::ArrowCursor);
}
