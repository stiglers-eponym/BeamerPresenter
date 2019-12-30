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

PreviewSlide::PreviewSlide(PdfDoc const * const document, int const pageNumber, QWidget* parent) :
    QWidget(parent),
    doc(document),
    cache(new CacheMap(document)),
    pageIndex(pageNumber)
{
    renderPage(pageNumber);
}

PreviewSlide::~PreviewSlide()
{
    // Clear all contents of the label.
    // This function is called when the document is reloaded or the program is closed and everything should be cleaned up.
    if (cache != nullptr)
        cache->clearCache();
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();
    page = nullptr;
}

void PreviewSlide::renderPage(int pageNumber)
{
    if (pageNumber < 0)
        pageNumber = 0;
    else if (pageNumber >= doc->getDoc()->numPages())
        pageNumber = doc->getDoc()->numPages()-1;

    // Use overlay specific options
    // A page is called an overlay of the previously rendered page, if they have the same label.
    // This is also the case, if the same page is rendered again (e.g. because the window is resized).
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();

    // Old cached images are useless if the label size has changed:
    if (size() != oldSize) {
        if (cache != nullptr)
            cache->clearCache();
        oldSize = size();
    }

    QPair<double,double> scale = basicRenderPage(pageNumber);
    pageIndex = pageNumber;

    // Show the page on the screen.
    // One could show the page in any case to make it slightly more responsive, but this can lead to a short interruption by a different image.
    // All operations before the next call to update() are usually very fast.
    update();

    // Collect link areas in pixels (positions relative to the lower left edge of the label)
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        linkPositions.append(QRect(
                    shiftx+int(relative.x()*scale.first),
                    shifty+int(relative.y()*scale.second),
                    int(relative.width()*scale.first),
                    int(relative.height()*scale.second)
                ));
    }
}

QPair<double,double> PreviewSlide::basicRenderPage(int const pageNumber)
{
    // Set the new page and basic properties
    page = doc->getPage(pageNumber);
    QSizeF pageSize = page->pageSizeF();
    // This is given in point = inch/72 ≈ 0.353mm (Did they choose these units to bother programmers?)

    // Place the page as an image of the correct size at the correct position
    // The lower left corner of the image will be located at (shiftx, shifty)
    double pageHeight=pageSize.height(), pageWidth=pageSize.width();
    // The page image must be split if the beamer option "notes on second screen" is set.
    if (pagePart != FullPage)
        pageWidth /= 2;
    // Check it width or height is the limiting constraint for the size of the displayed slide and calculate the resolution
    // resolution is calculated in pixels per point = dpi/72.
    if (width() * pageHeight > height() * pageWidth) {
        // the width of the label is larger than required
        resolution = double(height()) / pageHeight;
        shiftx = qint16(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        // the height of the label is larger than required
        resolution = double(width()) / pageWidth;
        shifty = qint16(height()/2 - resolution/2 * pageHeight);
        shiftx = 0;
    }
    if (cache != nullptr)
        cache->changeResolution(resolution);

    // Calculate the size of the image in pixels
    double scale_x=resolution*pageWidth, scale_y=resolution*pageHeight;
    // Adjustments if only parts of the page are shown:
    if (pagePart != FullPage) {
        scale_x *= 2;
        // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
        if (pagePart == RightHalf)
            shiftx -= width();
    }

    if (pageIndex != pageNumber && cache != nullptr)
        pixmap = cache->getPixmap(pageNumber);
    return {scale_x, scale_y};
}

void PreviewSlide::mouseReleaseEvent(QMouseEvent* event)
{
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
                    /*
                    case Poppler::Link::Rendition:
                        qInfo() << "Unsupported link of type rendition";
                        break;
                    case Poppler::Link::JavaScript:
                        qInfo() << "Unsupported link of type JavaScript";
                        break;
                    case Poppler::Link::OCGState: // requires popper >= 0.50
                        qInfo() << "Unsupported link of type OCGState";
                        break;
                    case Poppler::Link::Hide: // requires poppler >= 0.64
                        qInfo() << "Unsupported link of type hide";
                        break;
                    case Poppler::Link::None:
                        qInfo() << "Unsupported link of type none";
                        break;
                    */
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
    // Show the cursor as Qt::PointingHandCursor when hoovering links
    bool is_arrow_pointer = cursor().shape() == Qt::ArrowCursor;
    for (QList<QRect>::const_iterator pos_it=linkPositions.cbegin(); pos_it!=linkPositions.cend(); pos_it++) {
        if (pos_it->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    if (!is_arrow_pointer)
        setCursor(Qt::ArrowCursor);
    event->accept();
}

void PreviewSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(shiftx, shifty, pixmap);
}

void PreviewSlide::clearAll()
{
    if (cache != nullptr)
        cache->clearCache();
    qDeleteAll(links);
    links.clear();
    linkPositions.clear();
    page = nullptr;
}

int PreviewSlide::getCacheNumber() const
{
    if (cache == nullptr)
        return 0;
    return cache->length();
}

qint64 PreviewSlide::getCacheSize() const
{
    if (cache == nullptr)
        return 0;
    return cache->getSizeBytes();
}

QPixmap const PreviewSlide::getPixmap(int const page)
{
    if (cache == nullptr)
        return QPixmap();
    return cache->getPixmap(page);
}
