/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2020  stiglers-eponym

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

#ifndef PATHOVERLAY_H
#define PATHOVERLAY_H

#include <QWidget>
#include <QApplication>
#include "drawpath.h"
#include "../pdf/singlerenderer.h"

class DrawSlide;

class PathOverlay : public QWidget
{
    Q_OBJECT
    friend class DrawSlide;
    friend class PresentationSlide;

public:
    explicit PathOverlay(DrawSlide* parent);
    ~PathOverlay();

    void clearPageAnnotations();
    void clearAllAnnotations();
    void setSize(DrawTool const tool, quint16 size);

    QMap<QString, QList<DrawPath*>> const& getPaths() const {return paths;}
    ColoredDrawTool getTool() const {return tool;}
    quint16 getSize(DrawTool const tool) const {return sizes[tool];}

    /// Deprecated
    void saveDrawings(QString const& filename, QString const& notefile = "") const;
    /// Deprecated
    void loadDrawings(QString const& filename);
    /// Save files to compressed or uncompressed XML file.
    void saveXML(QString const& filename, PdfDoc const* notedoc, bool const compress = true) const;
    /// Load files from compressed or uncompressed XML file.
    void loadXML(QString const& filename);

    /// Set magnification factor for magnifier.
    void setMagnification(qreal const mag);
    /// Draw pointer or torch.
    void drawPointer(QPainter& painter);
    /// Move the last visible path to hidden paths.
    void undoPath();
    /// Move the last hidden path to visible paths.
    void redoPath();
    /// Reset cached pixmap of path overlays and update paths cache.
    void resetCache();
    /// Draw paths to painter (starting from end_cache).
    /// TODO: reorganize!
    void drawPaths(QPainter& painter, QString const label, bool const animation=false, bool const toCache=false);
    /// Does the given rectangle have any overlap with a video?
    bool hasVideoOverlap(QRectF const& rect) const;

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    /// Resize this widget and rescale all paths.
    void rescale(qint16 const oldshiftx, qint16 const oldshifty, double const oldRes);
    /// Erase paths at given point.
    void erase(QPointF const& point);
    /// Current draw tool.
    ColoredDrawTool tool = {NoTool, Qt::black};
    /// Currently visible paths.
    QMap<QString, QList<DrawPath*>> paths;
    /// Undisplayed paths which could be restored.
    QList<DrawPath*> undonePaths;
    /// Current position of the pointer.
    QPointF pointerPosition = QPointF();
    /// Page enlarged by magnification factor: used for magnifier.
    QPixmap enlargedPage;
    /// Renderer for enlarged page: enables rendering of enlarged page in separate thread.
    SingleRenderer* enlargedPageRenderer = nullptr;
    /// Sizes of tools.
    QMap<DrawTool, quint16> sizes = {{Magnifier,120}, {Torch,80}, {Pointer,10}, {Highlighter,30}, {Pen,3}, {Eraser,10}};
    /// Magnification factor for magnifier.
    qreal magnification = 2.;
    /// Pixmap containing only paths.
    QPixmap pixpaths;
    /// Index of last path (of current slide) which is already rendered to pixpaths.
    int end_cache = -1;
    /// Master slide to which this overlay is attached.
    DrawSlide const* master;

public slots:
    /// Update enlarged page (required for magnifier) if necessary.
    /// The page is rendered in a separate thread.
    void updateEnlargedPage();
    void setPaths(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setPathsQuick(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setPointerPosition(QPointF const point, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void setTool(ColoredDrawTool const newtool);
    void setTool(DrawTool const newtool, QColor const color=QColor()) {setTool({newtool, color});}
    void updatePathCache();
    void relax();
    void togglePointerVisibility();
    void showPointer();
    void hidePointer();

signals:
    void pointerPositionChanged(QPointF const point, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void pathsChangedQuick(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void pathsChanged(QString const pagelabel, QList<DrawPath*> const& list, qint16 const refshiftx, qint16 const refshifty, double const refresolution);
    void sendToolChanged(ColoredDrawTool const tool);
    void sendUpdateEnlargedPage();
    void sendRelax();
    void sendUpdatePathCache();
};

#endif // PATHOVERLAY_H
