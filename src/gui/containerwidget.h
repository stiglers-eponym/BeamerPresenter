// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QSize>
#include <QSizePolicy>
#include <QLayout>
#include "src/config.h"
#include "src/gui/containerbaseclass.h"
#include "src/gui/flexlayout.h"

/**
 * @brief  Widget for arangement of child widgets in QBoxLayout.
 *
 * Flexible container for widget arangement as read from json configuration file.
 *
 * @see TabWidget
 * @see StackedWidget
 * @see FlexLayout
 */
class ContainerWidget : public QWidget, public ContainerBaseClass
{
    Q_OBJECT

public:
    /// Constructor: initialize QWidget and GuiWidget.
    explicit ContainerWidget(QBoxLayout::Direction direction, QWidget *parent = nullptr) : QWidget(parent)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        FlexLayout* layout = new FlexLayout(direction);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
    }

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override
    {return layout() ? layout()->sizeHint() : QSize();}

    /// ContainerWidgets generally have a preferred aspect ratio. Thus hasHeightForWith()==true.
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Append a new widget to the layout.
    virtual void addWidgetCommon(QWidget *widget, const QString &title) override
    {if (layout()) layout()->addWidget(widget);}

    virtual QWidget *asWidget() noexcept override
    {return this;}
};

#endif // CONTAINERWIDGET_H
