#include "keyinputlabel.h"

KeyInputLabel::KeyInputLabel(const quint32 init, const Action action, QWidget *parent) :
    QLabel(parent),
    action(action),
    sequence(init)
{
    setText(QKeySequence(init).toString());
    setFocusPolicy(Qt::ClickFocus);
}

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
    const int new_sequence = event->key() | (event->modifiers() & ~Qt::KeypadModifier);
    event->accept();
    if (new_sequence == Qt::Key_Delete)
    {
        writable_preferences().key_actions.remove(sequence, action);
        setText("");
        sequence = 0;
    }
    else if (action || event->key())
    {
        setText(QKeySequence(new_sequence).toString());
        writable_preferences().key_actions.remove(sequence, action);
        writable_preferences().key_actions.insert(new_sequence, action);
        sequence = new_sequence;
    }
}
