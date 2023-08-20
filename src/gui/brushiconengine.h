// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef BRUSHICONENGINE_H
#define BRUSHICONENGINE_H

#include <QBrush>
#include <QColor>
#include <QRect>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QIconEngine>

/// Generate icon showing a single line with given width and style.
class BrushIconEngine : public QIconEngine
{
    /// Brush used to draw on the icon.
    QBrush brush;

public:
    /// Constructor: initialize brush.
    BrushIconEngine(Qt::BrushStyle style) : brush(Qt::black, style) {}

    /// Trivial destructor.
    ~BrushIconEngine() {}

    /// Draw horizontal line with given brush.
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State) override
    {painter->fillRect(rect, brush);}

    /// Clone this item.
    QIconEngine *clone() const override
    {return new BrushIconEngine(brush.style());}

    /// Draw transparent pixmap showing horizontal line with given brush.
    QPixmap pixmap(const QSize &size, QIcon::Mode, QIcon::State) override
    {
        QPixmap pixmap(size);
        pixmap.fill(QColor(0,0,0,0));
        QPainter painter;
        painter.begin(&pixmap);
        painter.fillRect(pixmap.rect(), brush);
        painter.end();
        return pixmap;
    }
};

#endif // BRUSHICONENGINE_H
