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

void KeyInputLabel::keyPressEvent(QKeyEvent *event)
{
    const int new_sequence = event->key() | (event->modifiers() & ~Qt::KeypadModifier);
    event->accept();
    if (new_sequence == Qt::Key_Delete)
    {
        writable_preferences().removeKeyAction(sequence, action);
        setText("");
        sequence = 0;
    }
    else if (action || event->key())
    {
        setText(QKeySequence(new_sequence).toString());
        writable_preferences().removeKeyAction(sequence, action);
        writable_preferences().addKeyAction(new_sequence, action);
        sequence = new_sequence;
    }
}
