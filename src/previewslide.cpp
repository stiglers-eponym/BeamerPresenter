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

PreviewSlide::~PreviewSlide()
{
    // Clear all contents of the label.
    // This function is called when the document is reloaded or the program is closed and everything should be cleaned up.
    clearCache();
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();
    page = nullptr;
}

void PreviewSlide::renderPage(Poppler::Page* page, QPixmap const* pix)
{
    if (page == nullptr)
        return;

    // Use overlay specific options
    // A page is called an overlay of the previously rendered page, if they have the same label.
    // This is also the case, if the same page is rendered again (e.g. because the window is resized).
    qDeleteAll(links);
    linkPositions.clear();
    links.clear();

    // Old cached images are useless if the label size has changed:
    if (size() != oldSize) {
        clearCache();
        oldSize = size();
    }

    // Set the new page and basic properties
    this->page = page;
    pageIndex = page->index();
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
        shiftx = int(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        // the height of the label is larger than required
        resolution = double(width()) / pageWidth;
        shifty = int(height()/2 - resolution/2 * pageHeight);
        shiftx = 0;
    }

    // Calculate the size of the image in pixels
    double scale_x=resolution*pageWidth, scale_y=resolution*pageHeight;
    // Adjustments if only parts of the page are shown:
    if (pagePart != FullPage) {
        scale_x *= 2;
        // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
        if (pagePart == RightHalf)
            shiftx -= width();
    }

    // Get the image
    if (pix != nullptr) {
        // A pixmap was passed to this function. Display this pixmap as the page image.
        if (pagePart != FullPage) {
            // The pixmap might show both notes and presentation.
            // Check the width to decide whether the image shows only the relevant part or the full page.
            if (pix->width() > 1.5*pixmap.width()) {
                // Assume that the pixmap shows notes and presentation.
                if (pagePart == LeftHalf)
                    pixmap = pix->copy(0, 0, pix->width()/2, pix->height());
                else
                    pixmap = pix->copy(pix->width()/2, 0, pix->width()/2, pix->height());
            }
            else
                pixmap = *pix;
        }
        else
            pixmap = *pix;
    }
    else {
        bool updateRequired = true;
        if (cache.contains(pageIndex)) {
            // The page exists in cache. Use the cache instead of rendering it again.
            pixmap = getCache(pageIndex);
            int picwidth = int(resolution*pageWidth), picheight = int(resolution*pageHeight);
            if (abs(picwidth-pixmap.width())<2 && abs(picheight-pixmap.height())<2)
                updateRequired = false;
        }
        if (updateRequired) {
            // A new page image has to be rendered.
            if (pagePart == FullPage)
                pixmap = QPixmap::fromImage(page->renderToImage(72*resolution, 72*resolution));
            else {
                QImage image = page->renderToImage(72*resolution, 72*resolution);
                if (pagePart == LeftHalf)
                    pixmap = QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
                else
                    pixmap = QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
            }
            // Save this image to cache.
            if (useCache)
                updateCache(&pixmap, page->index());
        }
    }

    // Show the page on the screen.
    // One could show the page in any case to make it slightly more responsive, but this can lead to a short interruption by a different image.
    // All operations before the next call to update() are usually very fast.
    update();

    // Collect link areas in pixels (positions relative to the lower left edge of the label)
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        linkPositions.append(QRect(
                    shiftx+int(relative.x()*scale_x),
                    shifty+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                ));
    }
}

long int PreviewSlide::updateCache(QPixmap const* pixmap, int const index)
{
    // Save the pixmap to (compressed) cache of page index and return the size of the compressed image.
    if (pixmap==nullptr || pixmap->isNull())
        return 0;
    // The image will be compressed and written to a QByteArray.
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap->save(&buffer, "PNG");
    cache[index] = bytes;
    return bytes->size();
}

long int PreviewSlide::updateCache(QByteArray const* bytes, int const index)
{
    // Write bytes to the cache of page index and return the size of bytes.
    if (bytes==nullptr || bytes->isNull() || bytes->isEmpty())
        return 0;
    else if (cache.contains(index))
        delete cache[index];
    cache[index] = bytes;
    return bytes->size();
}

long int PreviewSlide::updateCache(Poppler::Page const* cachePage)
{
    // Check whether the cachePage exists in cache. If yes, return 0.
    // Otherwise, render the given page using the internal renderer,
    // write the compressed image to cache and return the size of the compressed image.

    int index = cachePage->index();
    // Check whether the page exists in cache.
    if (cache.contains(index))
        return 0;

    // Render the page to a pixmap
    QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
    // if pagePart != FullPage: Reduce the image to the relevant part.
    if (pagePart == LeftHalf)
        image = image.copy(0, 0, image.width()/2, image.height());
    else if (pagePart == RightHalf)
        image = image.copy(image.width()/2, 0, image.width()/2, image.height());

    // This check is repeated, because it could be possible that the cache is overwritten while the image is rendered.
    if (cache.contains(index))
        return 0;

    // Write the image in png format to a QBytesArray
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    cache[index] = bytes;
    return bytes->size();
}

QPixmap PreviewSlide::getPixmap(Poppler::Page const* cachePage) const
{
    // Return a pixmap representing the current page.
    QPixmap pixmap;
    if (cache.contains(cachePage->index())) {
        // The page exists in cache. Use the cache instead of rendering it again.
        pixmap = getCache(cachePage->index());
        QSizeF size = page->pageSizeF();
        int picwidth = int(resolution*size.width()), picheight = int(resolution*size.height());
        if (abs(picwidth-pixmap.width())<2 && abs(picheight-pixmap.height())<2)
            return pixmap;
    }
    if (pagePart == FullPage)
        pixmap = QPixmap::fromImage(cachePage->renderToImage(72*resolution, 72*resolution));
    else {
        QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
        if (pagePart == LeftHalf)
            pixmap = QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
        else
            pixmap = QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
    }
    return pixmap;
}

QPixmap const PreviewSlide::getCache(int const index) const
{
    // Get a pixmap from cache.
    QPixmap pixmap;
    if (cache.contains(index)) {
        pixmap.loadFromData(*cache[index], "PNG");
        // If an external renderer is used, cached images always show the full page.
        // But if pagePart != FullPage, only one half of the image should be shown.
        if (pagePart != FullPage) {
            // The cached pixmap might show both notes and presentation.
            // Check the width to decide whether the image shows only the relevant part or the full page.
            if (pixmap.width() > 1.5*width()) {
                // Assume that the pixmap shows notes and presentation.
                if (pagePart == LeftHalf)
                    pixmap = pixmap.copy(0, 0, pixmap.width()/2, pixmap.height());
                else
                    pixmap = pixmap.copy(pixmap.width()/2, 0, pixmap.width()/2, pixmap.height());
            }
        }
    }
    return pixmap;
}

QByteArray const* PreviewSlide::getCachedBytes(int const index) const
{
    if (cache.contains(index))
        return cache[index];
    else
        return new QByteArray();
}

long int PreviewSlide::getCacheSize() const
{
    // Return the total size of all cached images of this label in bytes.
    long int size=0;
    for (QMap<int,QByteArray const*>::const_iterator it=cache.cbegin(); it!=cache.cend(); it++) {
        size += (*it)->size();
    }
    return size;
}

void PreviewSlide::clearCache()
{
    // Remove all images from cache.
    for (QMap<int,QByteArray const*>::const_iterator bytes=cache.cbegin(); bytes!=cache.cend(); bytes++) {
        delete *bytes;
    }
    cache.clear();
}

long int PreviewSlide::clearCachePage(const int index)
{
    // Delete the given page (page number index+1) from cache and return its size.
    // Return 0 if the page does not exist in cache.
    if (cache.contains(index)) {
        long int size = cache[index]->size();
        delete cache[index];
        cache.remove(index);
        return size;
    }
    else
        return 0;
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
                        qInfo() << "Playing multimedia is not supported in this widget.";
                        break;
                    case Poppler::Link::Movie:
                        qInfo() << "Unsupported link of type video.";
                        break;
                    case Poppler::Link::Rendition:
                        qInfo() << "Unsupported link of type rendition";
                        break;
                    case Poppler::Link::JavaScript:
                        qInfo() << "Unsupported link of type JavaScript";
                        break;
                    case Poppler::Link::OCGState:
                        qInfo() << "Unsupported link of type OCGState";
                        break;
                    case Poppler::Link::Hide:
                        qInfo() << "Unsupported link of type hide";
                        break;
                    case Poppler::Link::None:
                        qInfo() << "Unsupported link of type none";
                        break;
                }
            }
        }
    }
    event->accept();
}

void PreviewSlide::mouseMoveEvent(QMouseEvent* event)
{
    // Show the cursor as Qt::PointingHandCursor when hoovering links
    bool is_arrow_pointer = cursor() == Qt::ArrowCursor;
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
