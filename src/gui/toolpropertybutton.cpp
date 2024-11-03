// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/toolpropertybutton.h"

#include <QMouseEvent>
#include <QSize>
#include <QTabletEvent>
#include <QtConfig>

#include "src/drawing/tool.h"
#include "src/preferences.h"

ToolPropertyButton::ToolPropertyButton(QWidget *parent) : QComboBox(parent)
{
  setMinimumSize(12 + down_arrow_width, 12);
  setIconSize({24, 24});
  setStyleSheet("QComboBox{margin:0px;}QComboBox::drop-down{max-width:" +
                QString::number(down_arrow_width) +
                "px;}QAbstractItemView{min-width:4em;}");
  setFocusPolicy(Qt::NoFocus);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  setAttribute(Qt::WA_AcceptTouchEvents);
#if (QT_VERSION_MAJOR >= 6)
  connect(this, &QComboBox::activated, this, &ToolPropertyButton::changed);
#else
  connect(this, QOverload<int>::of(&QComboBox::activated), this,
          &ToolPropertyButton::changed);
#endif
}

bool ToolPropertyButton::event(QEvent *event)
{
  const int olddevice = device;
  switch (event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress: {
      const QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
      if (mouse_event->source() != Qt::MouseEventSynthesizedByQt)
        device = mouse_event->buttons() << 1;
      break;
    }
    case QEvent::MouseButtonRelease: {
      const QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
      if (mouse_event->source() != Qt::MouseEventSynthesizedByQt)
        device = mouse_event->button() << 1;
      break;
    }
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
      device = tablet_event_to_input_device(
          static_cast<const QTabletEvent *>(event));
      break;
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
      device = Tool::TouchInput;
      break;
    default:
      break;
  }
  if (device != olddevice) updateTool();
  return QComboBox::event(event);
}

void ToolPropertyButton::changed(const int index)
{
  std::shared_ptr<Tool> tool = preferences()->currentTool(device);
  setToolProperty(tool);
  emit sendUpdatedTool(tool);
}

void ToolPropertyButton::updateIcon()
{
  const int px = std::min(width() - down_arrow_width - 1, height()) - 1;
  setIconSize({px, px});
  update();
}
