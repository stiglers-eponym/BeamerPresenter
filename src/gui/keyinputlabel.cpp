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

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
    const int new_sequence = event->key() | (event->modifiers() & ~Qt::KeypadModifier);
    event->accept();
    if (new_sequence == Qt::Key_Delete)
    {
        writable_preferences()->removeKeyAction(sequence, action);
        setText("");
        sequence = 0;
    }
    else if (action || event->key())
    {
        setText(QKeySequence(new_sequence).toString());
        writable_preferences()->removeKeyAction(sequence, action);
        writable_preferences()->addKeyAction(new_sequence, action);
        sequence = new_sequence;
    }
}

void KeyInputLabel::changeAction(const QString &text) noexcept
{
    if (action != InvalidAction)
    {
        writable_preferences()->removeKeyAction(sequence, action);
        action = string_to_action_map.value(text, action);
        writable_preferences()->addKeyAction(sequence, action);
    }
    if (tool)
    {
        Tool *newtool = ToolDialog::selectTool(tool);
        writable_preferences()->replaceTool(tool, newtool);
    }
}
