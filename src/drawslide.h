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

#ifndef DRAWSLIDE_H
#define DRAWSLIDE_H

#include <QDataStream>
#include "mediaslide.h"
#include "drawpath.h"

// TODO: move pen strokes to different layer (extra widget) above multimedia widgets

class DrawSlide : public MediaSlide
{
    Q_OBJECT
public:
    explicit DrawSlide(QWidget* parent=nullptr) : MediaSlide(parent) {}
    explicit DrawSlide(PdfDoc const*const document, int const pageNumber, QWidget* parent=nullptr) : MediaSlide(document, pageNumber, parent) {}
    ~DrawSlide() override {clearAllAnnotations(); clearAll();}
    void clearPageAnnotations();
    void clearAllAnnotations();
    virtual void clearCache() override;
    void updateEnlargedPage();
    void setSize(DrawTool const tool, quint16 size);
    void setScaledPixmap(QPixmap const& pix);
    void setMagnification(qreal const mag);
    qreal getMagnification() const {return magnification;}

    QMap<QString, QList<DrawPath*>> const& getPaths() const {return paths;}
    double const& getResolution() const {return resolution;}
    ColoredDrawTool getTool() const {return tool;}
    quint16 getSize(DrawTool const tool) const {return sizes[tool];}
    void saveDrawings(QString const& filename, QString const& notefile = "") const;
    void loadDrawings(QString const& filename);

protected:
    void drawAnnotations(QPainter& painter);
    void drawPaths(QPainter& painter, QString const label, bool const clip=false);
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void animate(int const oldPageIndex = -1) override;
    void erase(QPointF const& point);
    ColoredDrawTool tool = {NoTool, Qt::black};
    QMap<QString, QList<DrawPath*>> paths;
    QPointF pointerPosition = QPointF();
    QPixmap enlargedPage;
    QMap<DrawTool, quint16> sizes = {{Magnifier,120}, {Torch,80}, {Pointer,10}, {Highlighter,30}, {Pen,3}, {Eraser,10}};
    bool pointer_visible = true;
    qreal magnification = 2.;
    QPixmap pixpaths;
    int end_cache = -1;

public slots:
    void setPaths(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setPathsQuick(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setPointerPosition(QPointF const point, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setTool(ColoredDrawTool const newtool);
    void setTool(DrawTool const newtool, QColor const color=QColor()) {setTool({newtool, color});}
    void updatePathCache();
    void relax();

signals:
    void pointerPositionChanged(QPointF const point, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void pathsChangedQuick(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void pathsChanged(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void sendToolChanged(ColoredDrawTool const tool);
    void sendUpdateEnlargedPage();
    void sendRelax();
    void sendUpdatePathCache();
};

#endif // DRAWSLIDE_H
