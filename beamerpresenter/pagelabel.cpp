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
    this->page = page;
    links = page->links();
    linkPositions = QList<QRect*>();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        QRect * absolute = new QRect(
                    int(relative.x()*window()->width()),
                    int(relative.y()*window()->height()),
                    int(relative.width()*window()->width()),
                    int(relative.height()*window()->height())
                );
        linkPositions.append( absolute );
    }
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
}

void PageLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            switch ( links.at(i)->linkType() )
            {
                case Poppler::Link::Goto:
                    if ( linkPositions.at(i)->contains(event->pos()) ) {
                        //std::cout << "Emit new page number signal" << std::endl;
                        emit sendNewPageNumber( ( (Poppler::LinkGoto*) links.at(i))->destination().pageNumber() - 1 );
                    }
                break;
                case Poppler::Link::Execute:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type execute" << std::endl;
                break;
                case Poppler::Link::Browse:
                    if ( linkPositions.at(i)->contains(event->pos()) )
                        std::cout << "Unsupported link of type browse" << std::endl;
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
