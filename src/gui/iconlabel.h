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

class IconLabel : public QLabel
{
    Q_OBJECT
    QImageReader *reader;

public:
    IconLabel(const QString &iconpath, QWidget *parent = nullptr)
        : QLabel(parent), reader(new QImageReader(iconpath, "svg")) {updateIcon();}
    ~IconLabel() {delete reader;}

public slots:
    void updateIcon()
    {
        const int px = std::min(width(), height())-1;
        reader->setFileName(reader->fileName()); // Workaround for a weird bug
        reader->setScaledSize({px, px});
        if (reader->canRead())
            setPixmap(QPixmap::fromImageReader(reader));
        else
            setText("?");
    }
};

#endif // ICONLABEL_H
