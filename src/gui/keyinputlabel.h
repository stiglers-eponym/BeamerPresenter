#ifndef KEYINPUTLABEL_H
#define KEYINPUTLABEL_H

#include <QKeyEvent>
#include <QLabel>
#include "src/preferences.h"

/**
 * @brief KeyInputLabel: QLabel for keyboard shortcut input.
 * @abstract Keyboard shortcuts registered by this widget are immediately
 * written to preferences.
 */
class KeyInputLabel : public QLabel
{
    Q_OBJECT
    Action action;
    quint32 sequence;

public:
    explicit KeyInputLabel(const quint32 init, const Action action, QWidget *parent = NULL);

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void changeAction(const QString &text) noexcept
    {action = string_to_action_map.value(text, action);}
};

#endif // KEYINPUTLABEL_H
