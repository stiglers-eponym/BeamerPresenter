// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QSize>
#include "src/gui/stackedwidget.h"

QSize StackedWidget::sizeHint() const noexcept
{
    QSize size(0,0);
    int i = 0;
    while (i < count())
        size = size.expandedTo(widget(i++)->sizeHint());
    return size;
}
