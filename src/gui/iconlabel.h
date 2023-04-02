// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <algorithm>
#include <QSize>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include "src/config.h"

class IconLabel : public QLabel
{
    Q_OBJECT

    /// Image reader for the icon.
    QIcon icon;

public:
    /// Constructor: Show icon.
    IconLabel(const QString &iconpath, QWidget *parent = nullptr)
        : QLabel(parent), icon(iconpath)
    {
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        updateIcon();
    }
    /// Trivial destructor.
    ~IconLabel() {}

public slots:
    /// Update icon: adjust icon to new size.
    void updateIcon()
    {
        if (icon.isNull())
            setText("?");
        else
            setPixmap(icon.pixmap(std::min(width(), height())-1));
    }
};

#endif // ICONLABEL_H
