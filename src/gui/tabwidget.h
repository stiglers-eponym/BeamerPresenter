// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QSizePolicy>
#include <QSize>
#include "src/config.h"

/**
 * @brief QTabWidget with adjusted size hint
 * @see StackedWidget
 * @see ContainerWidget
 */
class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    TabWidget(QWidget *parent = NULL) noexcept : QTabWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // TABWIDGET_H
