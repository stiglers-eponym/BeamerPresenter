// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>
#include <QSizePolicy>
#include <QSize>
#include "src/gui/containerbaseclass.h"
#include "src/config.h"

class QSize;

/**
 * @brief QStackedWidget with adjusted size hint
 * @see TabWidget
 * @see ContainerWidget
 */
class StackedWidget : public QStackedWidget, public ContainerBaseClass
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    StackedWidget(QWidget *parent = nullptr) noexcept : QStackedWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override
    {
        QSize size(0,0);
        int i = 0;
        while (i < count())
            size = size.expandedTo(widget(i++)->sizeHint());
        return size;
    }

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Append a new widget to the layout.
    virtual void addWidgetCommon(QWidget *widget, const QString &title) override
    {addWidget(widget);}

    virtual QWidget *asWidget() noexcept override
    {return this;}
};

#endif // STACKEDWIDGET_H
