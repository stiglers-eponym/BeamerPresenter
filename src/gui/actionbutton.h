#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QPushButton>
#include "src/enumerates.h"

/**
 * @brief Button which sends Action when clicked.
 */
class ActionButton : public QPushButton
{
    Q_OBJECT
    QSet<Action> actions;

public:
    explicit ActionButton(QWidget *parent = NULL);

    explicit ActionButton(const Action action, QWidget *parent = NULL) :
        ActionButton(parent) {addAction(action);}

    void addAction(const Action action)
    {if (action != InvalidAction) actions.insert(action);}

protected:
    void onClicked() const noexcept
    {for (const auto action : actions) emit sendAction(action);}

signals:
    void sendAction(const Action action) const;
};

#endif // ACTIONBUTTON_H
