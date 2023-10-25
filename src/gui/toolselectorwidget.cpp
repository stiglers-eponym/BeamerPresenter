// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/toolselectorwidget.h"

#include <QColor>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QResizeEvent>
#include <QSize>

#include "src/drawing/tool.h"
#include "src/gui/actionbutton.h"
#include "src/gui/brushstylebutton.h"
#include "src/gui/colorselectionbutton.h"
#include "src/gui/penstylebutton.h"
#include "src/gui/shapeselectionbutton.h"
#include "src/gui/toolselectorbutton.h"
#include "src/gui/widthselectionbutton.h"
#include "src/log.h"
#include "src/master.h"
#include "src/names.h"

ToolSelectorWidget::ToolSelectorWidget(QWidget *parent) : QWidget(parent)
{
  setContentsMargins(1, 1, 1, 1);
  QGridLayout *layout = new QGridLayout();
  setLayout(layout);
}

QSize ToolSelectorWidget::sizeHint() const noexcept
{
  QGridLayout *gridlayout = static_cast<QGridLayout *>(layout());
  return {gridlayout->columnCount() * 20, gridlayout->rowCount() * 20};
}

void ToolSelectorWidget::addButtons(const QJsonArray &full_array)
{
  QGridLayout *grid_layout = static_cast<QGridLayout *>(layout());
  for (int row_index = 0; row_index < full_array.size(); row_index++) {
    const QJsonArray row = full_array[row_index].toArray();
    for (int column_index = 0; column_index < row.size(); column_index++) {
      switch (row[column_index].type()) {
        case QJsonValue::String: {
          const QString &name = row[column_index].toString();
          const Action action = string_to_action_map.value(name, InvalidAction);
          if (action == InvalidAction)
            initializeToolPropertyButton(name, {}, row_index, column_index);
          else {
            ActionButton *button = new ActionButton(action, this);
            connect(this, &ToolSelectorWidget::updateIcons, button,
                    &ActionButton::updateIcon, Qt::QueuedConnection);
            if (button->icon().isNull()) button->setText(name);
            grid_layout->addWidget(button, row_index, column_index);
          }
          break;
        }
        case QJsonValue::Array: {
          QJsonArray array = row[column_index].toArray();
          if (array.isEmpty()) break;
          ActionButton *button = new ActionButton(this);
          connect(this, &ToolSelectorWidget::updateIcons, button,
                  &ActionButton::updateIcon, Qt::QueuedConnection);
          for (const auto &entry : array)
            button->addAction(
                string_to_action_map.value(entry.toString(), InvalidAction));
          if (button->icon().isNull())
            button->setText(array.first().toString());
          grid_layout->addWidget(button, row_index, column_index);
          break;
        }
        case QJsonValue::Object: {
          const QJsonObject obj = row[column_index].toObject();
          if (obj.contains("tool")) {
            Tool *tool = createTool(obj, 0);
            if (tool) {
              ToolSelectorButton *button = new ToolSelectorButton(tool, this);
              connect(this, &ToolSelectorWidget::updateIcons, button,
                      &ToolSelectorButton::updateIcon, Qt::QueuedConnection);
              grid_layout->addWidget(button, row_index, column_index);
            }
          } else if (obj.contains("select"))
            initializeToolPropertyButton(obj.value("select").toString(),
                                         obj.value("list").toArray(), row_index,
                                         column_index);
          else
            qWarning() << "Failed to create button"
                       << row[column_index].toObject();
          break;
        }
        default:
          break;
      }
    }
  }
}

void ToolSelectorWidget::initializeToolPropertyButton(const QString &type,
                                                      const QJsonArray &list,
                                                      const int row,
                                                      const int column)
{
  ToolPropertyButton *button{nullptr};
  if (type == "color")
    button = new ColorSelectionButton(list, this);
  else if (type == "width")
    button = new WidthSelectionButton(list, this);
  else if (type == "shape")
    button = new ShapeSelectionButton(this);
  else if (type == "style")
    button = new PenStyleButton(this);
  else if (type == "brush")
    button = new BrushStyleButton(this);
  if (button) {
    static_cast<QGridLayout *>(layout())->addWidget(button, row, column);
    connect(button, &ToolPropertyButton::sendToolProperties, this,
            &ToolSelectorWidget::sendToolProperties, Qt::DirectConnection);
    connect(master(), &Master::sendNewToolSoft, button,
            &ToolPropertyButton::toolChanged);
    connect(this, &ToolSelectorWidget::updateIcons, button,
            &ToolPropertyButton::updateIcon, Qt::QueuedConnection);
    connect(button, &ToolPropertyButton::sendUpdatedTool, this,
            &ToolSelectorWidget::updatedTool);
  }
}
