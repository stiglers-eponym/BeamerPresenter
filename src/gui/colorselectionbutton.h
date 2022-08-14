// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef COLORSELECTIONBUTTON_H
#define COLORSELECTIONBUTTON_H

#include <QJsonArray>
#include <QColor>
#include "src/gui/toolpropertybutton.h"

class Tool;

/**
 * @brief Drop down menu for changing the color of a tool.
 */
class ColorSelectionButton : public ToolPropertyButton
{
    Q_OBJECT
public:
    /// Constructor: add all items to the drop down menu.
    ColorSelectionButton(const QJsonArray &array, QWidget *parent = NULL);

    /// Trivial destructor.
    ~ColorSelectionButton() {}

protected:
    /// Set color of tool to selected value.
    void setToolProperty(Tool *tool) const override;

public slots:
    /// Update currently selected color based on tool.
    void toolChanged(Tool *tool) override;

signals:
    void colorChanged(const QColor &color) const;
};

#endif // COLORSELECTIONBUTTON_H
