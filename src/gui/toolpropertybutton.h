// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLPROPERTYBUTTON_H
#define TOOLPROPERTYBUTTON_H

#include <QComboBox>
#include "src/preferences.h"
#include "src/config.h"

class Tool;
class QEvent;

/**
 * @brief Drop down menu for changing a property of a tool.
 *
 * @see ToolSelectorWidget
 */
class ToolPropertyButton : public QComboBox
{
    Q_OBJECT
protected:
    /// Device changed by this button.
    /// This is the last device used to press this button.
    int device;

    /// Set device to the device producing this action, then continue with QComboBox::action.
    bool event(QEvent *event) override;

    /// Set property for given tool.
    virtual void setToolProperty(Tool* tool) const = 0;

    /// Update currently selected tool property based on device.
    virtual void updateTool()
    {toolChanged(preferences()->currentTool(device));}

public slots:
    /// Update currently selected tool property based on tool.
    virtual void toolChanged(const Tool *tool) = 0;

    /// Update the icon.
    virtual void updateIcon();

protected slots:
    /// Choose tool and call setToolProperty.
    void changed(const int index) const;

public:
    /// Constructor: adjust some widget properties.
    ToolPropertyButton(QWidget *parent = NULL);

    /// Trivial destructor.
    ~ToolPropertyButton() {}

signals:
    /// Notify master that tool has changed.
    void sendUpdatedTool(const Tool *tool) const;
};

#endif // TOOLPROPERTYBUTTON_H
