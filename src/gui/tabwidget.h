// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QSizePolicy>
#include <QSize>
#include "src/config.h"
#include "src/gui/containerbaseclass.h"


/**
 * @brief QTabWidget with adjusted size hint
 * @see StackedWidget
 * @see ContainerWidget
 */
class TabWidget : public QTabWidget, public ContainerBaseClass
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    TabWidget(QWidget *parent = nullptr) noexcept : QTabWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Append a new widget to the layout.
    virtual void addWidgetCommon(QWidget *widget, const QString &title) override
    {addTab(widget, title);}

    virtual QWidget *asWidget() noexcept override
    {return this;}
};

#endif // TABWIDGET_H
