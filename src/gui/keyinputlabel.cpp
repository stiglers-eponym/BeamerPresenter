#include "keyinputlabel.h"

KeyInputLabel::KeyInputLabel(const quint32 init, const Action action, QWidget *parent) :
    QLabel(parent),
    action(action),
    sequence(init)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setText(QKeySequence(init).toString());
    setBackgroundRole(QPalette::Base);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);
}

KeyInputLabel::KeyInputLabel(const quint32 init, Tool *tool, QWidget *parent) :
    QLabel(parent),
    tool(tool),
    sequence(init)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setText(QKeySequence(init).toString());
    setBackgroundRole(QPalette::Base);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);
}

KeyInputLabel::~KeyInputLabel()
{
    if (tool && !sequence)
        delete tool;
}

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
    const int new_sequence = event->key() | (event->modifiers() & ~Qt::KeypadModifier);
    event->accept();
    if (new_sequence == Qt::Key_Delete)
    {
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(sequence, action);
        if (tool)
            writable_preferences()->replaceKeyTool(sequence, tool);
        setText("");
        sequence = 0;
        return;
    }
    setText(QKeySequence(new_sequence).toString());
    if (action != InvalidAction)
    {
        writable_preferences()->removeKeyAction(sequence, action);
        writable_preferences()->addKeyAction(new_sequence, action);
    }
    if (tool)
    {
        writable_preferences()->replaceKeyToolShortcut(sequence, new_sequence, tool);
    }
    sequence = new_sequence;
}

void KeyInputLabel::changeAction(const QString &text) noexcept
{
    if (text == "tool...")
    {
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(sequence, action);
        Tool *newtool = ToolDialog::selectTool(tool);
        if (newtool)
        {
            writable_preferences()->replaceKeyTool(sequence, newtool);
            tool = newtool;
            action = InvalidAction;
        }
    }
    else
    {
        const Action newaction = string_to_action_map.value(text, action);
        if (newaction == InvalidAction)
            return;
        if (action != InvalidAction)
            writable_preferences()->removeKeyAction(sequence, action);
        action = newaction;
        writable_preferences()->addKeyAction(sequence, action);
        if (tool)
            writable_preferences()->replaceKeyTool(sequence, NULL);
        tool = NULL;
    }
}
