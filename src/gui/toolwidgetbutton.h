// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLWIDGETBUTTON_H
#define TOOLWIDGETBUTTON_H

#include "src/config.h"
#include "src/gui/toolbutton.h"

class ToolWidgetButton : public ToolButton
{
    Q_OBJECT
    const int device;

public:
    ToolWidgetButton(Tool *tool, const int device, QWidget *parent = nullptr) noexcept;
    virtual ~ToolWidgetButton() {}
protected slots:
    virtual void selectTool();
public slots:
    void receiveNewTool(const Tool *newtool);
};

#endif // TOOLWIDGETBUTTON_H
