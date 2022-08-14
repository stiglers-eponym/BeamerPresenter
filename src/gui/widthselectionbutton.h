// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef WIDTHSELECTIONBUTTON_H
#define WIDTHSELECTIONBUTTON_H

#include "src/gui/toolpropertybutton.h"

/**
 * @brief Drop down menu for changing the width of a draw tool.
 */
class WidthSelectionButton : public ToolPropertyButton
{
    Q_OBJECT
public:
    WidthSelectionButton(const QJsonArray &array, QWidget *parent = NULL);
    ~WidthSelectionButton() {}

protected:
    /// Set width of tool to selected value.
    void setToolProperty(Tool* tool) const override;

public slots:
    /// Update currently selected width based on tool.
    void toolChanged(Tool *tool) override;

signals:
    void widthChanged(const qreal width) const;
};

#endif // WIDTHSELECTIONBUTTON_H
