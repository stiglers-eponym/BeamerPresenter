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

bool operator<(ColoredDrawTool tool1, ColoredDrawTool tool2)
{
    // This function is required for sorting and searching in a QMap.
    // Does operaror<(QRgb, QRgb) together with == define a total order?
    return (tool1.tool<tool2.tool || (tool1.tool==tool2.tool && tool1.color.rgb()<tool2.color.rgb()) );
}


void DrawSlide::clearAllAnnotations()
{
    for (QMap<QString, QList<DrawPath*>>::iterator it=paths.begin(); it!=paths.end(); it++) {
        qDeleteAll(*it);
        it->clear();
    }
    paths.clear();
    update();
}

void DrawSlide::clearPageAnnotations()
{
    if (paths.contains(page->label())) {
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
    painter.drawPixmap(shiftx, shifty, pixmap);
    drawAnnotations(painter);
}

void DrawSlide::resizeEvent(QResizeEvent*)
{
    if (resolution < 0)
        return;
    enlargedPage = QPixmap();
    int const oldshiftx = shiftx, oldshifty = shifty;
    double const oldRes = resolution;
    QSizeF pageSize = page->pageSizeF();
    double pageWidth=pageSize.width();
    if (pagePart != FullPage)
        pageWidth /= 2;
    if (width() * pageSize.height() > height() * pageWidth) {
        resolution = double(height()) / pageSize.height();
        shiftx = int(width()/2 - resolution/2 * pageWidth);
        shifty = 0;
    }
    else {
        resolution = double(width()) / pageWidth;
        shifty = int(height()/2 - resolution/2 * pageSize.height());
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

void DrawSlide::setSize(DrawTool const tool, int size)
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

void DrawSlide::drawPaths(QPainter &painter, QString const label)
{
    if (paths.contains(label)) {
        for (QList<DrawPath*>::const_iterator path_it=paths[label].cbegin(); path_it!=paths[label].cend(); path_it++) {
            switch ((*path_it)->getTool()) {
            case Pen:
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen((*path_it)->getColor(), sizes[Pen]));
                painter.drawPolyline((*path_it)->data(), (*path_it)->number());
                break;
            case Highlighter:
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen((*path_it)->getColor(), sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                painter.drawPolyline((*path_it)->data(), (*path_it)->number());
                break;
            default:
                break;
            }
        }
    }
}

void DrawSlide::drawAnnotations(QPainter &painter)
{
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
                              QRectF(2*pointerPosition.x() - sizes[Magnifier], 2*pointerPosition.y() - sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier]));
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
    switch (event->buttons())
    {
    case Qt::LeftButton:
        // Left mouse button is used for painting.
        switch (tool.tool)
        {
        case None:
            return;
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
    // Middle mouse button always follows hyperlinks.
    switch (tool.tool) {
    case None:
    case Pointer:
        if (event->button() == Qt::LeftButton || event->button() == Qt::MidButton)
            followHyperlinks(event->pos());
        break;
    case Torch:
        pointerPosition = QPointF();
        update();
        break;
    case Magnifier:
        pointerPosition = QPointF();
        // Update enlarged page after eraser was used:
        if (event->button() == Qt::RightButton) {
            updateEnlargedPage();
            emit sendUpdateEnlargedPage();
        }
        update();
        break;
    case Pen:
    case Highlighter:
        paths[page->label()].last()->updateHash();
        break;
    default:
        break;
    }
    emit sendRelax();
    event->accept();
}

void DrawSlide::mouseMoveEvent(QMouseEvent *event)
{
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
        case None:
            if (pointer_visible)
                MediaSlide::mouseMoveEvent(event);
            break;
        case Pen:
        case Highlighter:
            if (paths.contains(page->label()) && paths[page->label()].length()>0) {
                paths[page->label()].last()->append(event->localPos());
                update();
                emit pathsChanged(page->label(), paths[page->label()], shiftx, shifty, resolution);
            }
            break;
        case Eraser:
            erase(event->localPos());
            update();
            break;
        case Torch:
        case Magnifier:
            pointerPosition = event->localPos();
            update();
            emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
            break;
        case Pointer:
            break;
        }
        break;
    case Qt::RightButton:
        erase(event->localPos());
        update();
        break;
    }
    event->accept();
}

void DrawSlide::erase(const QPointF &point)
{
    if (!paths.contains(page->label()))
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
            path_list.append(path_list[i]->split(0, splits.first()-1));
        for (int s=0; s<splits.size()-1; s++) {
            if (splits[s+1]-splits[s] > 3) {
                path_list.append(path_list[i]->split(splits[s]+1, splits[s+1]-1));
                s++;
            }
        }
        if (splits.last() < path_list[i]->number()-2)
            path_list.append(path_list[i]->split(splits.last()+1, path_list[i]->number()));
        delete path_list[i];
        path_list[i] = nullptr;
    }
    for (int i=0; i<oldsize && i<path_list.size();) {
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
    if (changed)
        emit pathsChanged(page->label(), path_list, shiftx, shifty, resolution);
}

void DrawSlide::clearCache()
{
    PreviewSlide::clearCache();
    enlargedPage = QPixmap();
}

void DrawSlide::setPaths(QString const pagelabel, QList<DrawPath*> const& list, int const refshiftx, int const refshifty, double const refresolution)
{
    QPointF shift = QPointF(shiftx, shifty) - resolution/refresolution*QPointF(refshiftx, refshifty);
    if (!paths.contains(pagelabel)) {
        paths[pagelabel] = QList<DrawPath*>();
        for (QList<DrawPath*>::const_iterator it = list.cbegin(); it!=list.cend(); it++)
            paths[pagelabel].append(new DrawPath(**it, shift, resolution/refresolution));
    }
    else {
        QMap<unsigned int, int> oldHashs;
        for (int i=0; i<paths[pagelabel].length(); i++)
            oldHashs[paths[pagelabel][i]->getHash()] = i;
        for (QList<DrawPath*>::const_iterator it = list.cbegin(); it!=list.cend(); it++) {
            if (oldHashs.remove((*it)->getHash()) == 0)
                paths[pagelabel].append(new DrawPath(**it, shift, resolution/refresolution));
        }
        QList<int> remove = oldHashs.values();
        std::sort(remove.begin(), remove.end());
        for (QList<int>::const_iterator it=remove.cend()-1; it!=remove.cbegin()-1; it--) {
            delete paths[pagelabel][*it];
            paths[pagelabel].removeAt(*it);
        }
    }
    update();
}

void DrawSlide::setPointerPosition(QPointF const point, int const refshiftx, int const refshifty, double const refresolution)
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
    if (tool.tool != Magnifier) {
        if (!enlargedPage.isNull())
            enlargedPage = QPixmap();
        return;
    }
    enlargedPage = QPixmap(2*size());
    enlargedPage.fill(QColor(0,0,0,0));
    QPainter painter;
    painter.begin(&enlargedPage);
    painter.drawImage(2*shiftx, 2*shifty, page->renderToImage(144*resolution, 144*resolution));
    painter.setRenderHint(QPainter::Antialiasing);
    if (paths.contains(page->label())) {
        for (QList<DrawPath*>::const_iterator path_it=paths[page->label()].cbegin(); path_it!=paths[page->label()].cend(); path_it++) {
            switch ((*path_it)->getTool()) {
            case Pen:
            {
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen((*path_it)->getColor(), 2*sizes[Pen]));
                DrawPath tmp(**path_it, QPointF(0,0), 2.);
                painter.drawPolyline(tmp.data(), tmp.number());
                break;
            }
            case Highlighter:
            {
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen((*path_it)->getColor(), 2*sizes[Highlighter]));
                DrawPath tmp(**path_it, QPointF(0,0), 2.);
                painter.drawPolyline(tmp.data(), tmp.number());
                break;
            }
            default:
                break;
            }
        }
    }
}
