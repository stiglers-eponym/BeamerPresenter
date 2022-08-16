// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QTabBar>
#include "src/gui/tabwidget.h"

QSize TabWidget::sizeHint() const noexcept
{
    QSize size(0,0);
    int i = 0;
    while (i < count())
        size = size.expandedTo(widget(i++)->sizeHint());
    switch (tabPosition())
    {
    case TabPosition::North:
    case TabPosition::South:
        size.rheight() += tabBar()->height();
        break;
    case TabPosition::West:
    case TabPosition::East:
        size.rwidth() += tabBar()->width();
        break;
    }
    return size;
}
