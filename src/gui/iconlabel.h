// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <algorithm>
#include <QSize>
#include <QLabel>
#include <QImageReader>
#include <QImage>
#include <QPixmap>
#include "src/config.h"

class QResizeEvent;

class IconLabel : public QLabel
{
    Q_OBJECT
    QImageReader *reader;

    void updateIcon()
    {
        const int px = std::min(width(), height());
        reader->setFileName(reader->fileName()); // Workaround for a weird bug
        reader->setScaledSize({px, px});
        if (reader->canRead())
            setPixmap(QPixmap::fromImageReader(reader));
        else
            setText("?");
    }

public:
    IconLabel(const QString &iconpath, QWidget *parent = nullptr)
        : QLabel(parent), reader(new QImageReader(iconpath, "svg")) {updateIcon();}
    ~IconLabel() {delete reader;}

protected:
    void resizeEvent(QResizeEvent *event) {updateIcon();}
};

#endif // ICONLABEL_H
