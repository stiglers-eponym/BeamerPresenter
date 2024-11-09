// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/toolbutton.h"

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QSize>
#include <QSizePolicy>
#include <QString>
#include <algorithm>

#include "src/config.h"
#include "src/gui/tooliconengine.h"

ToolButton::ToolButton(std::shared_ptr<Tool> tool, QWidget *parent) noexcept
    : QToolButton(parent), tool(nullptr)
{
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  setMinimumSize(16, 16);
  setIconSize({24, 24});
  setContentsMargins(0, 0, 0, 0);
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  if (tool) setTool(tool);
}

void ToolButton::setTool(std::shared_ptr<Tool> newtool)
{
  if (!newtool) return;
  tool = newtool;
  QIcon icon(new ToolIconEngine(tool));
  if (icon.isNull())
    setText(get_string_to_tool().key(tool->tool()));
  else
    setIcon(icon);
  setToolTip(Tool::tr(tool_to_description(tool->tool())));
}

void ToolButton::updateIcon()
{
  const int px = std::min(width(), height()) - 1;
  setIconSize({px, px});
  update();
}
