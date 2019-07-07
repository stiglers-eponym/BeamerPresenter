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
    if (paths.contains(pageIndex)) {
        paths[pageIndex].clear();
        update();
    }
}

void DrawSlide::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawPixmap(shiftx, shifty, pixmap);
}

void DrawSlide::drawAnnotations(QPainter &painter)
{
    if (paths.contains(pageIndex)) {
        painter.setRenderHint(QPainter::Antialiasing);
        if (paths[pageIndex].contains(RedPen)) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(QColor(255,0,0), 3));
            for (QList<DrawPath>::const_iterator it=paths[pageIndex][RedPen].cbegin(); it!=paths[pageIndex][RedPen].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
        if (paths[pageIndex].contains(GreenPen)) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(QColor(0,191,0), 3));
            for (QList<DrawPath>::const_iterator it=paths[pageIndex][GreenPen].cbegin(); it!=paths[pageIndex][GreenPen].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
        if (paths[pageIndex].contains(Highlighter)) {
            painter.setCompositionMode(QPainter::CompositionMode_Darken);
            painter.setPen(QPen(QColor(255,255,0), 30, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            for (QList<DrawPath>::const_iterator it=paths[pageIndex][Highlighter].cbegin(); it!=paths[pageIndex][Highlighter].cend(); it++)
                painter.drawPolyline(it->data(), it->number());
        }
    }
    // TODO: pointer, torch, ...
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
            if (!paths.contains(pageIndex) || !paths[pageIndex].contains(tool))
                paths[pageIndex][tool] = QList<DrawPath>();
            paths[pageIndex][tool].append(DrawPath(event->localPos()));
            break;
        case Erase:
            // TODO
            break;
        case Torch:
        case Pointer:
            // TODO
            break;
        case Magnifier:
            // TODO
            // render enlarged page
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
}

void DrawSlide::mouseReleaseEvent(QMouseEvent *event)
{
    // Middle mouse button always follows hyperlinks.
    if ((tool == None && event->button() == Qt::LeftButton) || event->button() == Qt::MidButton)
        followHyperlinks(event->pos());
    event->accept();
}

void DrawSlide::mouseMoveEvent(QMouseEvent *event)
{
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
            if (paths.contains(pageIndex) && paths[pageIndex].contains(tool) && paths[pageIndex][tool].length()>0) {
                paths[pageIndex][tool].last().append(event->localPos());
                update();
            }
            break;
        case Erase:
            erase(event->localPos());
            update();
            break;
        case Torch:
        case Pointer:
            // TODO
            break;
        case Magnifier:
            // TODO
            break;
        }
        break;
    case Qt::RightButton:
        erase(event->localPos());
        update();
        break;
    }
}

void DrawSlide::erase(const QPointF &point)
{
    if (!paths.contains(pageIndex))
        return;
    for (QMap<DrawTool, QList<DrawPath>>::iterator tool_it = paths[pageIndex].begin(); tool_it != paths[pageIndex].end(); tool_it++) {
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
}
