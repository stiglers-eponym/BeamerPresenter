#ifndef KEYINPUTLABEL_H
#define KEYINPUTLABEL_H

#include <QKeyEvent>
#include <QLabel>
#include "src/preferences.h"
#include "src/gui/tooldialog.h"

/**
 * @brief KeyInputLabel: QLabel for keyboard shortcut input.
 * @abstract Keyboard shortcuts registered by this widget are immediately
 * written to preferences. Tool selection via KeyInputLabel is not
 * implemented yet.
 */
class KeyInputLabel : public QLabel
{
    Q_OBJECT
    Action action = InvalidAction;
    Tool *tool = NULL;
    quint32 sequence;

public:
    explicit KeyInputLabel(const quint32 init, const Action action, QWidget *parent = NULL);
    explicit KeyInputLabel(const quint32 init, Tool *tool, QWidget *parent = NULL);
    ~KeyInputLabel();

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void changeAction(const QString &text) noexcept;
};

#endif // KEYINPUTLABEL_H
