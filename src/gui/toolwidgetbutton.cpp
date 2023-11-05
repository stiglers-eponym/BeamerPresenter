// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/toolwidgetbutton.h"

#include <QIcon>

#include "src/drawing/tool.h"
#include "src/gui/tooldialog.h"
#include "src/master.h"
#include "src/preferences.h"

ToolWidgetButton::ToolWidgetButton(std::shared_ptr<Tool> tool, const int device,
                                   QWidget *parent) noexcept
    : ToolButton(tool, parent), device(device)
{
  connect(this, &ToolWidgetButton::clicked, this,
          &ToolWidgetButton::selectTool);
  connect(master(), &Master::sendNewToolSoft, this,
          &ToolWidgetButton::receiveNewTool);
  if (!tool && (device & Tool::AnyActiveDevice))
    setTool(std::shared_ptr<Tool>(new Tool(Tool::NoTool, device)));
}

void ToolWidgetButton::selectTool()
{
  const std::shared_ptr<Tool> newtool = ToolDialog::selectTool(tool);
  if (newtool) {
    newtool->setDevice(newtool->device() | device);
    emit sendTool(newtool);
  }
}

void ToolWidgetButton::receiveNewTool(std::shared_ptr<const Tool> newtool)
{
  if (!newtool) return;
  if (newtool->device() & device) {
    std::shared_ptr<Tool> toolcopy = newtool->copy();
    toolcopy->setDevice(device);
    setTool(toolcopy);
  } else if (!preferences()->currentTool(device)) {
    if (!(device & Tool::AnyActiveDevice))
      setIcon(QIcon());
    else if (!tool || tool->tool() != Tool::NoTool)
      setTool(std::shared_ptr<Tool>(new Tool(Tool::NoTool, device)));
  }
}
