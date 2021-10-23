#include "src/gui/keyinputlabel.h"
#include "src/preferences.h"
#include "src/drawing/tool.h"
#include "src/gui/tooldialog.h"
#include "src/names.h"

KeyInputLabel::KeyInputLabel(const QKeySequence init, const Action action, QWidget *parent) :
    QLabel(parent),
    action(action),
    keys(init)
{
    setFrameStyle(int(QFrame::Panel) | QFrame::Sunken);
    setText(QKeySequence(init).toString());
    setBackgroundRole(QPalette::Base);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);
    setToolTip("Click here and press keyboard shortcut for this action or press delete to remove the shortcut.");
}

KeyInputLabel::KeyInputLabel(const QKeySequence init, Tool *tool, QWidget *parent) :
    QLabel(parent),
    tool(tool),
    keys(init)
{
    setFrameStyle(int(QFrame::Panel) | QFrame::Sunken);
    setText(QKeySequence(init).toString());
    setBackgroundRole(QPalette::Base);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);
    setToolTip("Click here and press keyboard shortcut for this tool or press delete to remove the shortcut.");
}

KeyInputLabel::~KeyInputLabel()
{
    if (tool)
    {
        writable_preferences()->removeKeyTool(tool, false);
        delete tool;
    }
}

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
    const QKeySequence new_keys(event->key() | (event->modifiers() & ~Qt::KeypadModifier));
    event->accept();
    if (new_keys == Qt::Key_Delete)
    {
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(keys, action);
        if (tool)
            writable_preferences()->replaceKeyToolShortcut(keys, 0, tool);
        setText("");
        keys = 0;
        return;
    }
    setText(new_keys.toString());
    if (action != InvalidAction)
    {
        writable_preferences()->removeKeyAction(keys, action);
        writable_preferences()->addKeyAction(new_keys, action);
    }
    if (tool)
        writable_preferences()->replaceKeyToolShortcut(keys, new_keys, tool);
    keys = new_keys;
}

void KeyInputLabel::changeAction(const QString &text) noexcept
{
    if (text == "tool...")
    {
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(keys, action);
        Tool *newtool = ToolDialog::selectTool(tool);
        if (newtool)
        {
            writable_preferences()->removeKeyTool(tool, true);
            delete tool;
            tool = newtool;
            action = InvalidAction;
            writable_preferences()->replaceKeyToolShortcut(0, keys, tool);
            emit sendName(string_to_tool.key(tool->tool()));
        }
    }
    else
    {
        const Action newaction = string_to_action_map.value(text, InvalidAction);
        if (newaction == InvalidAction)
        {
            // Invalid input detected. Reset input field.
            if (action != InvalidAction)
                emit sendName(string_to_action_map.key(action));
            else if (tool)
                emit sendName(string_to_tool.key(tool->tool()));
            else
                emit sendName("");
            return;
        }
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(keys, action);
        action = newaction;
        writable_preferences()->addKeyAction(keys, action);
        if (tool)
        {
            writable_preferences()->removeKeyTool(tool, true);
            delete tool;
            tool = NULL;
        }
    }
}
