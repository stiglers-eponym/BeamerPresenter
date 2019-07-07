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

void DrawSlide::clearAllAnnotations()
{
    paths.clear();
    update();
}

void DrawSlide::clearPageAnnotations()
{
    if (paths.contains(page->label())) {
        paths[page->label()].clear();
        update();
    }
}

void DrawSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(shiftx, shifty, pixmap);
    drawAnnotations(painter);
}

void DrawSlide::setTool(const DrawTool newtool)
{
    tool = newtool;
    if (pointer_visible) {
        setMouseTracking(true);
        setCursor(Qt::ArrowCursor);
    }
    else if (tool == Pointer)
        setMouseTracking(true);
    else
        setMouseTracking(false);
    if (tool == Magnifier) {
        pointerPosition = QPointF();
        if (enlargedPage.isNull() || enlargedPageNumber!=pageIndex) {
            enlargedPageNumber = pageIndex;
            enlargedPage = page->renderToImage(144*resolution, 144*resolution);
        }
    }
    else
        enlargedPage = QImage();
    if (tool == Torch)
        pointerPosition = QPointF();
}

void DrawSlide::togglePointerVisibility()
{
    if (pointer_visible) {
        pointer_visible = false;
        setCursor(Qt::BlankCursor);
        if (tool != Pointer)
            setMouseTracking(false);
    }
    else {
        pointer_visible = true;
        if (tool != Pointer)
            setCursor(Qt::ArrowCursor);
        setMouseTracking(true);
    }
}

void DrawSlide::setSize(const DrawTool tool, const int size)
{
    if (size < 1) {
        qWarning() << "Setting a tool size < 1 does not make any sense. Ignoring this.";
        return;
    }
    if (tool == Eraser) {
        for (QMap<QString, QMap<DrawTool, QList<DrawPath>>>::iterator page_it = paths.begin(); page_it != paths.end(); page_it++) {
            for (QMap<DrawTool, QList<DrawPath>>::iterator tool_it = page_it->begin(); tool_it != page_it->end(); tool_it++) {
                for (QList<DrawPath>::iterator path_it=tool_it->begin(); path_it!=tool_it->end(); path_it++) {
                    path_it->setEraserSize(size);
                }
            }
        }
    }
    sizes[tool] = size;
}

void DrawSlide::drawAnnotations(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    if (paths.contains(page->label())) {
        if (paths[page->label()].contains(GreenPen)) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(QColor(0,191,0), sizes[GreenPen]));
            for (QList<DrawPath>::const_iterator it=paths[page->label()][GreenPen].cbegin(); it!=paths[page->label()][GreenPen].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
        if (paths[page->label()].contains(RedPen)) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(QColor(255,0,0), sizes[RedPen]));
            for (QList<DrawPath>::const_iterator it=paths[page->label()][RedPen].cbegin(); it!=paths[page->label()][RedPen].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
        if (paths[page->label()].contains(Highlighter)) {
            painter.setCompositionMode(QPainter::CompositionMode_Darken);
            painter.setPen(QPen(QColor(255,255,0), sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            for (QList<DrawPath>::const_iterator it=paths[page->label()][Highlighter].cbegin(); it!=paths[page->label()][Highlighter].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
    }
    if (tool == Pointer) {
        painter.setCompositionMode(QPainter::CompositionMode_Darken);
        painter.setPen(QPen(QColor(255,0,0,191), sizes[Pointer], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPoint(pointerPosition);
    }
    else if (tool == Torch && !pointerPosition.isNull()) {
        QPainterPath rectpath;
        rectpath.addRect(shiftx, shifty, pixmap.width(), pixmap.height());
        QPainterPath circpath;
        circpath.addEllipse(pointerPosition, sizes[Torch], sizes[Torch]);
        painter.fillPath(rectpath-circpath, QColor(0,0,0,48));
    }
    else if (tool == Magnifier && !pointerPosition.isNull()) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setClipping(true);
        QPainterPath path;
        path.addEllipse(pointerPosition, sizes[Magnifier], sizes[Magnifier]);
        painter.setClipPath(path, Qt::ReplaceClip);
        //painter.setClipRegion(QRegion(pointerPosition.x()-sizes[Magnifier], pointerPosition.y()-sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier], QRegion::Ellipse), Qt::ClipOperation::ReplaceClip);
        painter.drawImage(QRectF(pointerPosition.x()-sizes[Magnifier], pointerPosition.y()-sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier]),
                          enlargedPage,
                          QRectF(2*(pointerPosition.x()-shiftx) - sizes[Magnifier], 2*(pointerPosition.y()-shifty) - sizes[Magnifier], 2*sizes[Magnifier], 2*sizes[Magnifier]));
        painter.setPen(QPen(QColor(32,32,32,64), 2));
        painter.drawEllipse(pointerPosition, sizes[Magnifier], sizes[Magnifier]);
    }
}

void DrawSlide::mousePressEvent(QMouseEvent *event)
{
    switch (event->buttons())
    {
    case Qt::LeftButton:
        // Left mouse button is used for painting.
        switch (tool)
        {
        case None:
            return;
        case RedPen:
        case GreenPen:
        case Highlighter:
            if (!paths.contains(page->label()) || !paths[page->label()].contains(tool))
                paths[page->label()][tool] = QList<DrawPath>();
            paths[page->label()][tool].append(DrawPath(event->localPos(), sizes[Eraser]));
            break;
        case Eraser:
            erase(event->localPos());
            update();
            break;
        case Torch:
        case Pointer:
        case Magnifier:
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
    if (((tool == None || tool == Pointer) && event->button() == Qt::LeftButton) || event->button() == Qt::MidButton)
        followHyperlinks(event->pos());
    else if (tool == Torch || tool == Magnifier) {
        pointerPosition = QPointF();
        update();
    }
    emit sendRelax();
    event->accept();
}

void DrawSlide::mouseMoveEvent(QMouseEvent *event)
{
    if (tool == Pointer) {
        pointerPosition = event->localPos();
        emit pointerPositionChanged(pointerPosition, shiftx, shifty, resolution);
        update();
    }
    switch (event->buttons())
    {
    case Qt::NoButton:
        MediaSlide::mouseMoveEvent(event);
        break;
    case Qt::LeftButton:
        switch (tool)
        {
        case None:
            MediaSlide::mouseMoveEvent(event);
            break;
        case RedPen:
        case GreenPen:
        case Highlighter:
            if (paths.contains(page->label()) && paths[page->label()].contains(tool) && paths[page->label()][tool].length()>0) {
                paths[page->label()][tool].last().append(event->localPos());
                update();
                emit pathsChanged(page->label(), tool, paths[page->label()][tool], shiftx, shifty, resolution);
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
    for (QMap<DrawTool, QList<DrawPath>>::iterator tool_it = paths[page->label()].begin(); tool_it != paths[page->label()].end(); tool_it++) {
        for (int i=0, length=tool_it->length(); i<length; i++) {
            QVector<int> splits = (*tool_it)[i].intersects(point);
            if (splits.isEmpty())
                continue;
            if (splits.first() > 1)
                tool_it->append((*tool_it)[i].split(0, splits.first()-1));
            for (int s=0; s<splits.size()-1; s++) {
                if (splits[s+1]-splits[s] > 3) {
                    tool_it->append((*tool_it)[i].split(splits[s]+1, splits[s+1]-1));
                    s++;
                }
            }
            if (splits.last() < (*tool_it)[i].number()-2)
                tool_it->append((*tool_it)[i].split(splits.last()+1, (*tool_it)[i].number()));
            (*tool_it)[i].clear();
        }
        for (int i=0; i<tool_it->length(); i++) {
            if ((*tool_it)[i].isEmpty())
                tool_it->removeAt(i);
        }
    }
    emit pathsChanged(page->label(), GreenPen, paths[page->label()][GreenPen], shiftx, shifty, resolution);
    emit pathsChanged(page->label(), RedPen, paths[page->label()][RedPen], shiftx, shifty, resolution);
    emit pathsChanged(page->label(), Highlighter, paths[page->label()][Highlighter], shiftx, shifty, resolution);
}

void DrawSlide::clearCache()
{
    PreviewSlide::clearCache();
    enlargedPage = QImage();
}

void DrawSlide::setPaths(QString const page, DrawTool const tool, QList<DrawPath> const& list, int const xshift, int const yshift, double const resolution)
{
    if (paths.contains(page) && paths[page].contains(tool))
        paths[page][tool].clear();
    else
        paths[page][tool] = QList<DrawPath>();
    QPointF shift = QPointF(this->shiftx, this->shifty) - this->resolution/resolution*QPointF(xshift, yshift);
    for (QList<DrawPath>::const_iterator it = list.cbegin(); it!=list.cend(); it++)
        paths[page][tool].append(DrawPath(*it, shift, this->resolution/resolution));
    update();
}

void DrawSlide::setPointerPosition(QPointF const point, int const xshift, int const yshift, double const resolution)
{
    pointerPosition = (point - QPointF(xshift, yshift)) * this->resolution/resolution + QPointF(this->shiftx, this->shifty);
    if (tool == Pointer || tool == Magnifier || tool == Torch)
        update();
}

void DrawSlide::relax()
{
    if (tool == Torch || tool == Magnifier) {
        pointerPosition = QPointF();
        update();
    }
}
