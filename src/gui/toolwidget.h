// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLWIDGET_H
#define TOOLWIDGET_H

#include <QWidget>
#include "src/drawing/tool.h"
#include "src/config.h"

class QSize;
class Tool;

class ToolWidget : public QWidget
{
    Q_OBJECT

    int devices {0};
    Qt::Orientation orientation;

    void initialize();
    void addDeviceGroup(const QList<Tool::InputDevice> &new_devices);

public:
    explicit ToolWidget(QWidget *parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);

    /// Size hint for layout.
    QSize sizeHint() const noexcept override;

    /// Optimal height depends on width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

public slots:
    void checkNewTool(const Tool *tool);

signals:
    void receiveTool(const Tool *tool);
    void sendTool(Tool *tool);
};

const QString get_device_icon(int device) noexcept;

#endif // TOOLWIDGET_H
