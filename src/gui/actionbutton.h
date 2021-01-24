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
    const Action action;

public:
    explicit ActionButton(const Action action, QWidget *parent = NULL);

protected:
    void onClicked() const noexcept
    {sendAction(action);}

signals:
    void sendAction(const Action action) const;
};

#endif // ACTIONBUTTON_H
