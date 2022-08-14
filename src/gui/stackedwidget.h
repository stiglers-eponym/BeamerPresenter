// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>
#include "src/config.h"

/**
 * @brief QStackedWidget with adjusted size hint
 * @see TabWidget
 * @see ContainerWidget
 */
class StackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    StackedWidget(QWidget *parent = NULL) noexcept : QStackedWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // STACKEDWIDGET_H
