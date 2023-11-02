// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/keyinputlabel.h"

#include <QKeyEvent>
#include <QPalette>
#include <QtConfig>

#include "src/drawing/tool.h"
#include "src/gui/settingswidget.h"
#include "src/gui/tooldialog.h"
#include "src/names.h"
#include "src/preferences.h"

KeyInputLabel::KeyInputLabel(const QKeySequence init, const Action action,
                             QWidget *parent)
    : QLabel(parent), action(action), keys(init)
{
  setFrameStyle(int(QFrame::Panel) | QFrame::Sunken);
  setText(QKeySequence(init).toString());
  setBackgroundRole(QPalette::Base);
  QPalette palette = QPalette();
  palette.setColor(QPalette::Base, Qt::white);
  setPalette(palette);
  setAutoFillBackground(true);
  setFocusPolicy(Qt::ClickFocus);
  setToolTip(
      tr("Click here and press keyboard shortcut for this action or press "
         "delete to remove the shortcut."));
}

KeyInputLabel::KeyInputLabel(const QKeySequence init,
                             std::shared_ptr<Tool> tool, QWidget *parent)
    : QLabel(parent), tool(tool), keys(init)
{
  setFrameStyle(int(QFrame::Panel) | QFrame::Sunken);
  setText(QKeySequence(init).toString());
  setBackgroundRole(QPalette::Base);
  QPalette palette = QPalette();
  palette.setColor(QPalette::Base, Qt::white);
  setPalette(palette);
  setAutoFillBackground(true);
  setFocusPolicy(Qt::ClickFocus);
  setToolTip(
      tr("Click here and press keyboard shortcut for this tool or press delete "
         "to remove the shortcut."));
}

KeyInputLabel::~KeyInputLabel()
{
  if (tool) writable_preferences()->removeKeyTool(tool, false);
}

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
#if (QT_VERSION_MAJOR >= 6)
  const QKeySequence new_keys(event->keyCombination());
#else
  const QKeySequence new_keys(event->key() |
                              (event->modifiers() & ~Qt::KeypadModifier));
#endif
  event->accept();
  if (new_keys == Qt::Key_Delete) {
    if (action != InvalidAction)
      writable_preferences()->removeKeyAction(keys, action);
    if (tool) writable_preferences()->replaceKeyToolShortcut(keys, 0, tool);
    setText("");
    keys = 0;
    return;
  }
  setText(new_keys.toString());
  if (action != InvalidAction) {
    writable_preferences()->removeKeyAction(keys, action);
    writable_preferences()->addKeyAction(new_keys, action);
  }
  if (tool)
    writable_preferences()->replaceKeyToolShortcut(keys, new_keys, tool);
  keys = new_keys;
}

void KeyInputLabel::changeAction(const QString &text) noexcept
{
  const auto box = dynamic_cast<const QComboBox *>(sender());
  if (!box) return;
  const Action newaction = box->currentData().value<Action>();
  if (newaction == InvalidAction) {
    if (text != SettingsWidget::tr("tool...")) {
      // Invalid input detected. Reset input field.
      if (action != InvalidAction)
        emit sendName(SettingsWidget::tr(
            string_to_action_map.key(action).toLatin1().constData()));
      else if (tool)
        emit sendName(
            Tool::tr(string_to_tool.key(tool->tool()).toLatin1().constData()));
      else
        emit sendName("");
      return;
    }
    if (action != InvalidAction)
      writable_preferences()->removeKeyAction(keys, action);
    std::shared_ptr<Tool> newtool = ToolDialog::selectTool(tool);
    if (newtool) {
      if (tool) writable_preferences()->removeKeyTool(tool, true);
      tool = newtool;
      action = InvalidAction;
      writable_preferences()->replaceKeyToolShortcut(0, keys, tool);
      emit sendName(
          Tool::tr(string_to_tool.key(tool->tool()).toLatin1().constData()));
    }
  } else {
    if (action != InvalidAction)
      writable_preferences()->removeKeyAction(keys, action);
    action = newaction;
    writable_preferences()->addKeyAction(keys, action);
    if (tool) {
      writable_preferences()->removeKeyTool(tool, true);
      tool = nullptr;
    }
  }
}
