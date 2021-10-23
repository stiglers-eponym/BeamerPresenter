#ifndef KEYINPUTLABEL_H
#define KEYINPUTLABEL_H

#include "src/enumerates.h"
#include <QLabel>
#include <QKeyEvent>

class Tool;

/**
 * @brief KeyInputLabel: QLabel for keyboard shortcut input.
 * @abstract Keyboard shortcuts registered by this widget are immediately
 * written to preferences. Tool selection via KeyInputLabel is not
 * implemented yet.
 */
class KeyInputLabel : public QLabel
{
    Q_OBJECT
    /// Key combinations entered here will be connected to this Action if it is valid.
    Action action = InvalidAction;
    /// Key combinations entered here will be connected to this Tool if it is not NULL.
    Tool *tool = NULL;
    /// key sequence
    QKeySequence keys;

public:
    /// Create a new KeyInputLabel for an action.
    explicit KeyInputLabel(const QKeySequence init, const Action action, QWidget *parent = NULL);
    /// Create a new KeyInputLabel for a tool.
    explicit KeyInputLabel(const QKeySequence init, Tool *tool, QWidget *parent = NULL);
    ~KeyInputLabel();

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void changeAction(const QString &text) noexcept;

signals:
    void sendName(const QString &string);
};

#endif // KEYINPUTLABEL_H
