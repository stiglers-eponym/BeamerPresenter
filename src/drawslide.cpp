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

#include "drawslide.h"

/// This function is required for sorting and searching in a QMap.
bool operator<(ColoredDrawTool tool1, ColoredDrawTool tool2)
{
    // TODO: Does operaror<(QRgb, QRgb) together with == define a total order?
    return (tool1.tool<tool2.tool || (tool1.tool==tool2.tool && tool1.color.rgb()<tool2.color.rgb()) );
}


void DrawSlide::clearAllAnnotations()
{
    for (QMap<QString, QList<DrawPath*>>::iterator it=paths.begin(); it!=paths.end(); it++) {
        qDeleteAll(*it);
        it->clear();
    }
    paths.clear();
    end_cache = -1;
    if (!pixpaths.isNull())
        pixpaths = QPixmap();
    update();
}

void DrawSlide::clearPageAnnotations()
{
    end_cache = -1;
    if (!pixpaths.isNull())
        pixpaths = QPixmap();
    if (page != nullptr && paths.contains(page->label())) {
        qDeleteAll(paths[page->label()]);
        paths[page->label()].clear();
        update();
        if (tool.tool == Magnifier)
            updateEnlargedPage();
    }
}

void DrawSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (pixpaths.isNull() || end_cache < 1)
        painter.drawPixmap(shiftx, shifty, pixmap);
    else
        painter.drawPixmap(0, 0, pixpaths);
    drawAnnotations(painter);
}

void DrawSlide::resizeEvent(QResizeEvent*)
{
    if (resolution < 0 || page == nullptr)
        return;
    enlargedPage = QPixmap();
    qint16 const oldshiftx = shiftx, oldshifty = shifty;
    double const oldRes = resolution;
    QSizeF pageSize = page->pageSizeF();
    double pageWidth=pageSize.width();
    if (pagePart != FullPage)
        pageWidth /= 2;
    if (width() * pageSize.height() > height() * pageWidth) {
        resolution = double(height()) / pageSize.height();
        shiftx = qint16(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        resolution = double(width()) / pageWidth;
        shifty = qint16(height()/2 - resolution/2 * pageSize.height());
        shiftx = 0;
    }
    QPointF shift = QPointF(shiftx, shifty) - resolution/oldRes*QPointF(oldshiftx, oldshifty);
    for (QMap<QString, QList<DrawPath*>>::iterator page_it = paths.begin(); page_it != paths.end(); page_it++)
        for (QList<DrawPath*>::iterator path_it = page_it->begin(); path_it != page_it->end(); path_it++)
            (*path_it)->transform(shift, resolution/oldRes);
}

void DrawSlide::setScaledPixmap(QPixmap const& pix)
{
    if (shiftx == 0)
        pixmap = pix.scaledToWidth(width());
    else
        pixmap = pix.scaledToHeight(height());
}

void DrawSlide::setTool(const ColoredDrawTool newtool)
{
    tool = newtool;
    // TODO: fancy cursors
    if (!pointer_visible && tool.tool != Pointer)
        setMouseTracking(false);
    if (tool.tool == Torch) {
        enlargedPage = QPixmap();
        pointerPosition = QPointF();
    }
    else if (tool.tool == Pointer) {
        enlargedPage = QPixmap();
        setMouseTracking(true);
        if (underMouse()) {
            pointerPosition = mapFromGlobal(QCursor::pos());
            emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
        }
    }
    else if (tool.tool == Magnifier) {
        pointerPosition = QPointF();
        if (enlargedPage.isNull())
            updateEnlargedPage();
    }
    else
        enlargedPage = QPixmap();
    update();
}

void DrawSlide::updatePathCache()
{
    if (page == nullptr)
        return;
    if (paths[page->label()].isEmpty()) {
        end_cache = -1;
        if (!pixpaths.isNull())
            pixpaths = QPixmap();
    }
    else {
        if (pixpaths.isNull())
            end_cache = -1;
        if (end_cache == -1) {
            pixpaths = QPixmap(size());
            pixpaths.fill(palette().base().color());
        }
        QPainter painter;
        painter.begin(&pixpaths);
        if (end_cache == -1)
            painter.drawPixmap(shiftx, shifty, pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        drawPaths(painter, page->label());
        end_cache = paths[page->label()].length();
    }
}

void DrawSlide::setSize(DrawTool const tool, quint16 size)
{
    if (size < 1)
        size = 1;
    if (tool == Eraser) {
        for (QMap<QString, QList<DrawPath*>>::iterator page_it = paths.begin(); page_it != paths.end(); page_it++)
            for (QList<DrawPath*>::iterator path_it=page_it->begin(); path_it!=page_it->end(); path_it++)
                (*path_it)->setEraserSize(size);
    }
    sizes[tool] = size;
}

void DrawSlide::drawPaths(QPainter &painter, QString const label, bool const clip)
{
    if (page == nullptr)
        return;
    if (paths.contains(label)) {
        QList<DrawPath*>::const_iterator path_it = paths[label].cbegin();
        if (label == page->label() && end_cache > 0)
            path_it += end_cache;
        for (; path_it!=paths[label].cend(); path_it++) {
            switch ((*path_it)->getTool()) {
            case Pen:
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen((*path_it)->getColor(), sizes[Pen]));
                painter.drawPolyline((*path_it)->data(), (*path_it)->number());
                break;
            case Highlighter:
                if (clip)
                    painter.setClipRect(shiftx, shifty, width()-2*shiftx, height()-2*shifty);
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen((*path_it)->getColor(), sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter.drawPolyline((*path_it)->data(), (*path_it)->number());
                if (clip)
                    painter.setClipRect(rect());
                break;
            default:
                break;
            }
        }
    }
}

void DrawSlide::drawAnnotations(QPainter &painter)
{
    if (page == nullptr)
        return;
    painter.setRenderHint(QPainter::Antialiasing);
    drawPaths(painter, page->label());
    switch (tool.tool) {
    case Pointer:
        painter.setCompositionMode(QPainter::CompositionMode_Darken);
        painter.setPen(QPen(tool.color, sizes[Pointer], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPoint(pointerPosition);
        break;
    case Torch:
        if (!pointerPosition.isNull()) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            QPainterPath rectpath;
            rectpath.addRect(shiftx, shifty, pixmap.width(), pixmap.height());
            QPainterPath circpath;
            circpath.addEllipse(pointerPosition, sizes[Torch], sizes[Torch]);
            painter.fillPath(rectpath-circpath, tool.color);
        }
        break;
    case Magnifier:
        if (!pointerPosition.isNull()) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setClipping(true);
            QPainterPath path;
            path.addEllipse(pointerPosition, sizes[Magnifier], sizes[Magnifier]);
            painter.setClipPath(path, Qt::ReplaceClip);
            painter.drawPixmap(QRectF(pointerPosition.x()-sizes[Magnifier], pointerPosition.y()-sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier]),
                              enlargedPage,
                              QRectF(magnification*pointerPosition.x() - sizes[Magnifier], magnification*pointerPosition.y() - sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier]));
            painter.setPen(QPen(tool.color, 2));
            painter.drawEllipse(pointerPosition, sizes[Magnifier], sizes[Magnifier]);
        }
        break;
    default:
        break;
    }
}

void DrawSlide::mousePressEvent(QMouseEvent *event)
{
    if (page == nullptr)
        return;
    switch (event->buttons())
    {
    case Qt::LeftButton:
        // Left mouse button is used for painting.
        switch (tool.tool)
        {
        case Pen:
        case Highlighter:
            if (!paths.contains(page->label()))
                paths[page->label()] = QList<DrawPath*>();
            paths[page->label()].append(new DrawPath(tool, event->localPos(), sizes[Eraser]));
            break;
        case Eraser:
            erase(event->localPos());
            update();
            break;
        case Magnifier:
            if (enlargedPage.isNull())
                updateEnlargedPage();
        [[clang::fallthrough]];
        case Torch:
        case Pointer:
            pointerPosition = event->localPos();
            update();
            emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
            break;
        default:
            return;
        }
        break;
    case Qt::RightButton:
        erase(event->localPos());
        update();
        break;
    default:
        break;
    }
    event->accept();
}

void DrawSlide::mouseReleaseEvent(QMouseEvent *event)
{
    if (page == nullptr)
        return;
    switch (event->button())
    {
    case Qt::RightButton:
        updatePathCache();
        emit sendUpdatePathCache();
        if (tool.tool == Magnifier) {
            updateEnlargedPage();
            emit sendUpdateEnlargedPage();
            update();
        }
        break;
    case Qt::LeftButton:
        switch (tool.tool) {
        case NoTool:
        case Pointer:
            followHyperlinks(event->pos());
            break;
        case Torch:
        case Magnifier:
            pointerPosition = QPointF();
            update();
            break;
        case Pen:
        case Highlighter:
        case Eraser:
            updatePathCache();
            emit sendUpdatePathCache();
            break;
        default:
            break;
        }
        break;
    case Qt::MidButton:
        followHyperlinks(event->pos());
        break;
    default:
        break;
    }
    emit sendRelax();
    event->accept();
}

void DrawSlide::mouseMoveEvent(QMouseEvent *event)
{
    if (page == nullptr)
        return;
    if (tool.tool == Pointer) {
        pointerPosition = event->localPos();
        emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
        update();
    }
    switch (event->buttons())
    {
    case Qt::NoButton:
        if (pointer_visible)
            MediaSlide::mouseMoveEvent(event);
        break;
    case Qt::LeftButton:
        switch (tool.tool)
        {
        case Pen:
        case Highlighter:
            if (!paths[page->label()].isEmpty()) {
                paths[page->label()].last()->append(event->localPos());
                update();
                emit pathsChangedQuick(page->label(), paths[page->label()], shiftx, shifty, resolution);
            }
            break;
        case Eraser:
            erase(event->localPos());
            break;
        case Torch:
        case Magnifier:
            pointerPosition = event->localPos();
            update();
            emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
            break;
        case Pointer:
            break;
        default:
            if (pointer_visible)
                MediaSlide::mouseMoveEvent(event);
        }
        break;
    case Qt::RightButton:
        erase(event->localPos());
        break;
    }
    event->accept();
}

void DrawSlide::erase(const QPointF &point)
{
    if (page == nullptr || paths[page->label()].isEmpty())
        return;
    QList<DrawPath*>& path_list = paths[page->label()];
    int const oldsize=path_list.size();
    bool changed = false;
    for (int i=0; i<oldsize; i++) {
        QVector<int> splits = path_list[i]->intersects(point);
        if (splits.isEmpty())
            continue;
        changed = true;
        if (splits.first() > 1)
            path_list.insert(i+1, path_list[i]->split(0, splits.first()-1));
        for (int s=0; s<splits.size()-1; s++) {
            if (splits[s+1]-splits[s] > 3) {
                path_list.insert(i+1, path_list[i]->split(splits[s]+1, splits[s+1]-1));
                s++;
            }
        }
        if (splits.last() < path_list[i]->number()-2)
            path_list.insert(i+1, path_list[i]->split(splits.last()+1, path_list[i]->number()));
        delete path_list[i];
        path_list[i] = nullptr;
    }
    for (int i=0; i<path_list.size();) {
        if (path_list[i] == nullptr)
            path_list.removeAt(i);
        //else if (path_list[i]->isEmpty()) { // this should never happen...
        //    qDebug() << "this should no happen.";
        //    delete path_list[i];
        //    path_list.removeAt(i);
        //}
        else
            i++;
    }
    if (changed) {
        end_cache = -1;
        repaintIfPresentation();
        emit pathsChanged(page->label(), path_list, shiftx, shifty, resolution);
    }
}

void DrawSlide::clearCache()
{
    PreviewSlide::clearCache();
    enlargedPage = QPixmap();
    end_cache = -1;
    pixpaths = QPixmap();
}

void DrawSlide::setPathsQuick(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution)
{
    QPointF shift = QPointF(shiftx, shifty) - resolution/refresolution*QPointF(refshiftx, refshifty);
    int const diff = list.length() - paths[pagelabel].length();
    if (diff == 0) {
        if (!paths[pagelabel].last()->update(*list.last(), shift, resolution/refresolution))
            setPaths(pagelabel, list, refshiftx, refshifty, refresolution);
    }
    else if (diff == 1)
        paths[pagelabel].append(new DrawPath(*list.last(), shift, resolution/refresolution));
    else if (diff < 0) {
        if (-diff >= paths[pagelabel].length()) {
            qDeleteAll(paths[pagelabel]);
            paths[pagelabel].clear();
        }
        else {
            for (int i=diff; i++<0;)
                delete paths[pagelabel].takeLast();
        }
        end_cache = -1;
        pixpaths = QPixmap();
    }
    else {
        qDebug() << "set paths quick failed!" << this;
        setPaths(pagelabel, list, refshiftx, refshifty, refresolution);
    }
    repaintIfPresentation();
}

void DrawSlide::setPaths(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution)
{
    QPointF shift = QPointF(shiftx, shifty) - resolution/refresolution*QPointF(refshiftx, refshifty);
    if (!paths.contains(pagelabel)) {
        paths[pagelabel] = QList<DrawPath*>();
        for (QList<DrawPath*>::const_iterator it = list.cbegin(); it!=list.cend(); it++)
            paths[pagelabel].append(new DrawPath(**it, shift, resolution/refresolution));
    }
    else {
        // Basic assumption: If list and paths[pagelabel] both contain two elements, then these elements appear in the same order in both lists.
        QList<DrawPath*>::const_iterator new_it=list.cbegin();
        QList<DrawPath*>::iterator old_it=paths[pagelabel].begin();
        for (;new_it<list.cend() && old_it<paths[pagelabel].end() && (*new_it)->getHash() == (*old_it)->getHash(); new_it++, old_it++) {}
        if (old_it >= paths[pagelabel].end()-1) {
            if (old_it == paths[pagelabel].end()-1) {
                delete *old_it;
                paths[pagelabel].pop_back();
            }
            while (new_it < list.cend())
                paths[pagelabel].append(new DrawPath(**(new_it++), shift, resolution/refresolution));
        }
        else {
            // create look up table for new hashs
            QMap<quint32, QList<DrawPath*>::const_iterator> newHashs;
            for (QList<DrawPath*>::const_iterator it=new_it; it<list.cend(); it++)
                newHashs[(*it)->getHash()] = it;
            // adjust paths
            for (QList<DrawPath*>::const_iterator next; old_it<paths[pagelabel].end();) {
                // next new path which exists already in the old paths:
                next = newHashs.value((*old_it)->getHash(), list.cend());
                if (next == list.cend()) {
                    delete *old_it;
                    old_it = paths[pagelabel].erase(old_it);
                }
                else {
                    while (new_it < next)
                        old_it = paths[pagelabel].insert(old_it, new DrawPath(**(new_it++), shift, resolution/refresolution)) + 1;
                    new_it++;
                    old_it++;
                }
            }
            while (new_it < list.cend())
                paths[pagelabel].append(new DrawPath(**(new_it++), shift, resolution/refresolution));
        }
    }
    end_cache = -1;
    updatePathCache();
    repaintIfPresentation();
}

void DrawSlide::setPointerPosition(QPointF const point, qint16 const refshiftx, qint16 const refshifty, double const refresolution)
{
    pointerPosition = (point - QPointF(refshiftx, refshifty)) * resolution/refresolution + QPointF(shiftx, shifty);
    if (tool.tool == Magnifier && enlargedPage.isNull())
        updateEnlargedPage();
    if (tool.tool == Pointer || tool.tool == Magnifier || tool.tool == Torch)
        update();
}

void DrawSlide::relax()
{
    if (tool.tool == Torch || tool.tool == Magnifier) {
        pointerPosition = QPointF();
        update();
    }
}

void DrawSlide::updateEnlargedPage()
{
    if (tool.tool != Magnifier || page == nullptr) {
        if (!enlargedPage.isNull())
            enlargedPage = QPixmap();
        return;
    }
    enlargedPage = QPixmap(magnification*size());
    enlargedPage.fill(QColor(0,0,0,0));
    QPainter painter;
    painter.begin(&enlargedPage);
    painter.drawImage(int(magnification*shiftx), int(magnification*shifty), page->renderToImage(72*magnification*resolution, 72*magnification*resolution));
    painter.setRenderHint(QPainter::Antialiasing);
    if (paths.contains(page->label())) {
        for (QList<DrawPath*>::const_iterator path_it=paths[page->label()].cbegin(); path_it!=paths[page->label()].cend(); path_it++) {
            switch ((*path_it)->getTool()) {
            case Pen:
            {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen((*path_it)->getColor(), magnification*sizes[Pen]));
                DrawPath tmp(**path_it, QPointF(0,0), magnification);
                painter.drawPolyline(tmp.data(), tmp.number());
                break;
            }
            case Highlighter:
            {
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen((*path_it)->getColor(), magnification*sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                DrawPath tmp(**path_it, QPointF(0,0), magnification);
                painter.drawPolyline(tmp.data(), tmp.number());
                break;
            }
            default:
                break;
            }
        }
    }
}

void DrawSlide::saveDrawings(QString const& filename, QString const& notefile) const
{
    // Save drawings in a strange data format.
    qInfo() << "Saving files is experimental. Files might contain errors or might be unreadable for later versions of BeamerPresenter";
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    stream  << static_cast<quint32>(0x2CA7D9F8)
            << static_cast<quint16>(stream.version())
            << QFileInfo(doc->getPath()).absoluteFilePath()
            << QFileInfo(notefile).absoluteFilePath();
    if (stream.status() == QDataStream::WriteFailed) {
        qCritical() << "Failed to write to file: File is not writable.";
        return;
    }
    {
        QMap<quint16, quint16> newsizes;
        for (QMap<DrawTool, quint16>::const_iterator size_it=sizes.cbegin(); size_it!=sizes.cend(); size_it++)
            newsizes[static_cast<quint16>(size_it.key())] = *size_it;
        stream << newsizes;
    }
    stream << static_cast<quint16>(paths.size());
    qint16 const w = qint16(width()-2*shiftx), h = qint16(height()-2*shifty);
    for (QMap<QString, QList<DrawPath*>>::const_iterator page_it=paths.cbegin(); page_it!=paths.cend(); page_it++) {
        stream << page_it.key() << static_cast<quint16>(page_it->length());
        for (QList<DrawPath*>::const_iterator path_it=page_it->cbegin(); path_it!=page_it->cend(); path_it++) {
            QVector<float> vec;
            (*path_it)->toIntVector(vec, shiftx, shifty, w, h);
            stream << static_cast<quint16>((*path_it)->getTool()) << (*path_it)->getColor() << vec;
        }
    }
    if (stream.status() != QDataStream::Ok) {
        qCritical() << "Error occurred while writing to file.";
        return;
    }
}

void DrawSlide::loadDrawings(QString const& filename)
{
    // Load drawings from the strange data format.
    qInfo() << "Loading files is experimental. Files might contain errors or might be unreadable for later versions of BeamerPresenter";
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << "Loading file failed: file does not exist.";
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Loading file failed: file is not readable.";
        return;
    }
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    {
        quint32 magic;
        stream >> magic;
        if (magic != 0x2CA7D9F8) {
            qCritical() << "Invalid file: wrong magic number.";
            return;
        }
    }
    {
        quint16 version;
        stream >> version;
        stream.setVersion(version);
    }
    {
        QString docpath, notepath;
        stream >> docpath >> notepath;
        if (stream.status() != QDataStream::Ok) {
            qCritical() << "Failed to load file: File is corrupt.";
            return;
        }
        if (docpath != doc->getPath())
            qWarning() << "This drawing file was generated for a different PDF file path.";
    }
    {
        QMap<quint16, quint16> newsizes;
        stream >> newsizes;
        if (stream.status() != QDataStream::Ok) {
            qCritical() << "Failed to load file: File is corrupt.";
            return;
        }
        for (QMap<quint16, quint16>::const_iterator size_it=newsizes.cbegin(); size_it!=newsizes.cend(); size_it++)
            sizes[static_cast<DrawTool>(size_it.key())] = *size_it;
    }
    quint16 npages, npaths;
    stream >> npages;
    if (stream.status() != QDataStream::Ok) {
        qCritical() << "Failed to load file: File is corrupt.";
        return;
    }
    clearAllAnnotations();
    qint16 const w = qint16(width()-2*shiftx), h = qint16(height()-2*shifty);
    QString pagelabel;
    quint16 tool;
    QColor color;
    for (int pageidx=0; pageidx<npages; pageidx++) {
        stream >> pagelabel >> npaths;
        if (stream.status() != QDataStream::Ok) {
            qCritical() << "Interrupted reading file: File is corrupt.";
            break;
        }
        paths[pagelabel] = QList<DrawPath*>();
        for (int pathidx=0; pathidx<npaths; pathidx++) {
            QVector<float> vec;
            stream >> tool >> color >> vec;
            if (stream.status() != QDataStream::Ok) {
                qCritical() << "Interrupted reading file: File is corrupt.";
                break;
            }
            paths[pagelabel].append(new DrawPath({static_cast<DrawTool>(tool), color}, vec, shiftx, shifty, w, h, sizes[Eraser]));
        }
        emit pathsChanged(pagelabel, paths[pagelabel], shiftx, shifty, resolution);
    }
    update();
}

void DrawSlide::setMagnification(qreal const mag)
{
    if (mag <= 0.) {
        qWarning() << "Cannot set magnification to negative value";
        return;
    }
    magnification = mag;
    if (!enlargedPage.isNull()) {
        enlargedPage = QPixmap();
        updateEnlargedPage();
    }
}

void DrawSlide::animate(const int oldPageIndex)
{
     if (oldPageIndex != pageIndex) {
         end_cache = -1;
         updatePathCache();
     }
}
