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

#include "previewslide.h"

PreviewSlide::PreviewSlide(PdfDoc const * const document, PagePart const part, QWidget* parent) :
    QWidget(parent),
    doc(document),
    cache(new CacheMap(document, part)),
    pagePart(part),
    pageIndex(0)
{
    //setAttribute(Qt::WA_OpaquePaintEvent);
}

PreviewSlide::~PreviewSlide()
{
    if (cache != nullptr)
        cache->clearCache();
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();
    page = nullptr;
}

void PreviewSlide::renderPage(int pageNumber)
{
#ifdef DEBUG_RENDERING
    qDebug() << "preview slide render page" << pageNumber << this;
#endif
    // Make sure that pageNumber is valid.
    if (pageNumber < 0)
        pageNumber = 0;
    else if (pageNumber >= doc->getDoc()->numPages())
        pageNumber = doc->getDoc()->numPages()-1;

    // Use overlay specific options.
    // A page is called an overlay of the previously rendered page, if they have the same label.
    // This is also the case, if the same page is rendered again (e.g. because the window is resized).

    // Clear links.
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();

    // Do the main rendering. This returns a pair of scales in x an y direction.
    // These scale relative x and y coordinates in the widget to pixels in the pixmap representing the slide.
    QSizeF const scale = basicRenderPage(pageNumber);
    // Update pageIndex.
    pageIndex = pageNumber;

    // Show the page on the screen.
    // All operations before the next call to update() are usually very fast.
    update();

    // Collect link areas in pixels (positions relative to the lower left edge of the label)
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        linkPositions.append(QRect(
                    shiftx+int(relative.x()*scale.width()),
                    shifty+int(relative.y()*scale.height()),
                    int(relative.width()*scale.width()),
                    int(relative.height()*scale.height())
                ));
    }
}

QSizeF const PreviewSlide::basicRenderPage(int const pageNumber)
{
#ifdef DEBUG_RENDERING
    qDebug() << "basic render page" << size() << this;
#endif
    // Set the new page and basic properties
    page = doc->getPage(pageNumber);
    // This is given in point = inch/72 ≈ 0.353mm (Did they choose these units to bother programmers?)
    QSizeF pageSize = page->pageSizeF();

    // Place the page as an image of the correct size at the correct position
    // The lower left corner of the image will be located at (shiftx, shifty)
    // The page image must be split if the beamer option "notes on second screen" is set.
    if (pagePart != FullPage)
        pageSize.rwidth() /= 2;
    // Check it width or height is the limiting constraint for the size of the displayed slide and calculate the resolution
    // resolution is calculated in pixels per point = dpi/72.
    if (width() * pageSize.height() > height() * pageSize.width()) {
        // the width of the label is larger than required
        resolution = qreal(height()) / pageSize.height();
        shiftx = qint16(width()/2 - resolution/2 * pageSize.width());
        shifty = 0;
    }
    else {
        // the height of the label is larger than required
        resolution = qreal(width()) / pageSize.width();
        shifty = qint16(height()/2 - resolution/2 * pageSize.height());
        shiftx = 0;
    }
    if (cache != nullptr) {
        // Change the resolution on the CacheMap.
        // This clears cache if the resolution differs from the resolution saved in cache.
        cache->changeResolution(resolution);
    }

    // Calculate the size of the image in pixels
    qreal scale_x=resolution*pageSize.width(), scale_y=resolution*pageSize.height();
    // Adjustments if only parts of the page are shown:
    if (pagePart != FullPage) {
        scale_x *= 2;
        // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
        if (pagePart == RightHalf)
            shiftx -= width();
    }

    // Render the pixmap if necessary.
#ifdef DEBUG_RENDERING
    qDebug() << "get pixmap?" << pageIndex << pageNumber << oldSize << size() << cache << this;
#endif
    // Check whether the page number or the widget size changed. Then update pixmap if cache is available.
    if ((pageIndex != pageNumber || oldSize != size()) && cache != nullptr)
        pixmap = cache->getPixmap(pageNumber);
    // Update size. This will later be used to check it the pixmap needs to be updated.
    oldSize = size();
        return {scale_x, scale_y};
}

void PreviewSlide::mouseReleaseEvent(QMouseEvent* event)
{
    // Handle clicks on links.
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            if (linkPositions[i].contains(event->pos())) {
                switch ( links[i]->linkType() )
                {
                    case Poppler::Link::Goto:
                        if (static_cast<Poppler::LinkGoto*>(links[i])->isExternal()) {
                            // Link to an other document
                            QString filename = static_cast<Poppler::LinkGoto*>(links[i])->fileName();
                            QDesktopServices::openUrl(QUrl(filename, QUrl::TolerantMode));
                        }
                        else {
                            // Link to an other page
                            emit sendNewPageNumber( static_cast<Poppler::LinkGoto*>(links[i])->destination().pageNumber() - 1 );
                        }
                        return;
                    case Poppler::Link::Execute:
                        // Handle execution links, which are marked for execution as an embedded application.
                        // In this case, a corresponding item has been added to embeddedWidgets in renderPage.
                        {
                            Poppler::LinkExecute* link = static_cast<Poppler::LinkExecute*>(links[i]);
                            QStringList splitFileName = QStringList();
                            if (!urlSplitCharacter.isEmpty())
                                splitFileName = link->fileName().split(urlSplitCharacter);
                            else
                                splitFileName.append(link->fileName());
                            QUrl url = QUrl(splitFileName[0], QUrl::TolerantMode);
                            // TODO: handle arguments
                            QDesktopServices::openUrl(url);
                        }
                        break;
                    case Poppler::Link::Browse:
                        // Link to file or website
                        QDesktopServices::openUrl( QUrl(static_cast<Poppler::LinkBrowse*>(links[i])->url(), QUrl::TolerantMode) );
                        break;
                    case Poppler::Link::Action:
                        {
                            Poppler::LinkAction* link = static_cast<Poppler::LinkAction*>(links[i]);
                            switch (link->actionType())
                            {
                                case Poppler::LinkAction::Quit:
                                case Poppler::LinkAction::Close:
                                    emit sendCloseSignal();
                                    return;
                                case Poppler::LinkAction::Print:
                                    qInfo() << "Unsupported link action: print.";
                                    break;
                                case Poppler::LinkAction::GoToPage:
                                    emit focusPageNumberEdit();
                                    break;
                                case Poppler::LinkAction::PageNext:
                                    emit sendNewPageNumber(pageIndex + 1);
                                    return;
                                case Poppler::LinkAction::PagePrev:
                                    emit sendNewPageNumber(pageIndex - 1);
                                    return;
                                case Poppler::LinkAction::PageFirst:
                                    emit sendNewPageNumber(0);
                                    return;
                                case Poppler::LinkAction::PageLast:
                                    emit sendNewPageNumber(-1);
                                    return;
                                case Poppler::LinkAction::Find:
                                    // TODO: implement this
                                    qInfo() << "Unsupported link action: find.";
                                    break;
                                case Poppler::LinkAction::Presentation:
                                    // untested
                                    emit sendShowFullscreen();
                                    break;
                                case Poppler::LinkAction::EndPresentation:
                                    // untested
                                    emit sendEndFullscreen();
                                    break;
                                case Poppler::LinkAction::HistoryBack:
                                    // TODO: implement this
                                    qInfo() << "Unsupported link action: history back.";
                                    break;
                                case Poppler::LinkAction::HistoryForward:
                                    // TODO: implement this
                                    qInfo() << "Unsupported link action: history forward.";
                                    break;
                            }
                        }
                        break;
                    case Poppler::Link::Sound:
                    case Poppler::Link::Movie:
                        qInfo() << "Playing multimedia is not supported in this widget.";
                        break;
                    default:
                        qInfo() << "Unsupported link type" << links[i]->linkType();
                }
            }
        }
    }
    event->accept();
}

void PreviewSlide::mouseMoveEvent(QMouseEvent* event)
{
    // Show the cursor as Qt::PointingHandCursor when hovering links
    bool is_arrow_pointer = cursor().shape() == Qt::ArrowCursor;
    // Iterate over all link position to check whether a link position contains the cursor position.
    for (QList<QRect>::const_iterator pos_it=linkPositions.cbegin(); pos_it!=linkPositions.cend(); pos_it++) {
        if (pos_it->contains(event->pos())) {
            // Cursor is on a link. Set it to PointingHandCursor and return.
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            event->accept();
            return;
        }
    }
    // Cursor is not on a link. Set it to ArrowCursor.
    if (!is_arrow_pointer)
        setCursor(Qt::ArrowCursor);
    event->accept();
}

void PreviewSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (pagePart == RightHalf)
        painter.drawPixmap(shiftx + width(), shifty, pixmap);
    else
        painter.drawPixmap(shiftx, shifty, pixmap);
}

void PreviewSlide::clearAll()
{
    // Clear cache (if it exists).
    if (cache != nullptr)
        cache->clearCache();
    // Delete all links and link positions.
    qDeleteAll(links);
    links.clear();
    linkPositions.clear();
    // Set page to nullptr.
    page = nullptr;
}

QPixmap const PreviewSlide::getPixmap(int const page)
{
    if (cache == nullptr)
        return QPixmap();
    return cache->getPixmap(page);
}
