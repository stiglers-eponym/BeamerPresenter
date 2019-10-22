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
    paths.clear();
    update();
}

void DrawSlide::clearPageAnnotations()
{
    if (paths.contains(page->label())) {
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
    for (QMap<QString, QMap<ColoredDrawTool, QList<DrawPath>>>::iterator page_it = paths.begin(); page_it != paths.end(); page_it++)
        for (QMap<ColoredDrawTool, QList<DrawPath>>::iterator tool_it = page_it->begin(); tool_it != page_it->end(); tool_it++)
            for (QList<DrawPath>::iterator path_it = tool_it->begin(); path_it != tool_it->end(); path_it++)
                path_it->transform(shift, resolution/oldRes);
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

void DrawSlide::setSize(const DrawTool tool, const int size)
{
    if (size < 1) {
        qWarning() << "Setting a tool size < 1 does not make any sense. Ignoring this.";
        return;
    }
    if (tool == Eraser) {
        for (QMap<QString, QMap<ColoredDrawTool, QList<DrawPath>>>::iterator page_it = paths.begin(); page_it != paths.end(); page_it++) {
            for (QMap<ColoredDrawTool, QList<DrawPath>>::iterator tool_it = page_it->begin(); tool_it != page_it->end(); tool_it++) {
                for (QList<DrawPath>::iterator path_it=tool_it->begin(); path_it!=tool_it->end(); path_it++) {
                    path_it->setEraserSize(size);
                }
            }
        }
    }
    sizes[tool] = size;
}

void DrawSlide::drawPaths(QPainter &painter, QString const label)
{
    if (paths.contains(label)) {
        // TODO: put this in an own function
        for (QMap<ColoredDrawTool, QList<DrawPath>>::const_iterator map_it=paths[label].cbegin(); map_it!=paths[label].cend(); map_it++) {
            switch (map_it.key().tool) {
            case Pen:
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(map_it.key().color, sizes[Pen]));
                for (QList<DrawPath>::const_iterator path_it=map_it->cbegin(); path_it!=map_it->cend(); path_it++)
                    painter.drawPolyline(path_it->data(), path_it->number());
                break;
            case Highlighter:
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen(map_it.key().color, sizes[Highlighter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                for (QList<DrawPath>::const_iterator path_it=map_it->cbegin(); path_it!=map_it->cend(); path_it++)
                    painter.drawPolyline(path_it->data(), path_it->number());
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
            if (!paths.contains(page->label()) || !paths[page->label()].contains(tool))
                paths[page->label()][tool] = QList<DrawPath>();
            paths[page->label()][tool].append(DrawPath(event->localPos(), sizes[Eraser]));
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
    for (QMap<ColoredDrawTool, QList<DrawPath>>::iterator tool_it = paths[page->label()].begin(); tool_it != paths[page->label()].end(); tool_it++) {
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
        emit pathsChanged(page->label(), tool_it.key(), tool_it.value(), shiftx, shifty, resolution);
    }
}

void DrawSlide::clearCache()
{
    PreviewSlide::clearCache();
    enlargedPage = QPixmap();
}

void DrawSlide::setPaths(QString const pagelabel, ColoredDrawTool const tool, QList<DrawPath> const& list, int const refshiftx, int const refshifty, double const refresolution)
{
    if (paths.contains(pagelabel) && paths[pagelabel].contains(tool))
        paths[pagelabel][tool].clear();
    else
        paths[pagelabel][tool] = QList<DrawPath>();
    QPointF shift = QPointF(shiftx, shifty) - resolution/refresolution*QPointF(refshiftx, refshifty);
    for (QList<DrawPath>::const_iterator it = list.cbegin(); it!=list.cend(); it++)
        paths[pagelabel][tool].append(DrawPath(*it, shift, resolution/refresolution));
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
        for (QMap<ColoredDrawTool, QList<DrawPath>>::const_iterator map_it=paths[page->label()].cbegin(); map_it!=paths[page->label()].cend(); map_it++) {
            switch (map_it.key().tool) {
            case Pen:
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setPen(QPen(map_it.key().color, 2*sizes[Pen]));
                for (QList<DrawPath>::const_iterator path_it=map_it->cbegin(); path_it!=map_it->cend(); path_it++) {
                    DrawPath tmp(*path_it, QPointF(0,0), 2.);
                    painter.drawPolyline(tmp.data(), tmp.number());
                }
                break;
            case Highlighter:
                painter.setCompositionMode(QPainter::CompositionMode_Darken);
                painter.setPen(QPen(map_it.key().color, 2*sizes[Highlighter]));
                for (QList<DrawPath>::const_iterator path_it=map_it->cbegin(); path_it!=map_it->cend(); path_it++) {
                    DrawPath tmp(*path_it, QPointF(0,0), 2.);
                    painter.drawPolyline(tmp.data(), tmp.number());
                }
                break;
            default:
                break;
            }
        }
    }
}
