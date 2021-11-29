#ifndef TOOLSELECTORWIDGET_H
#define TOOLSELECTORWIDGET_H

#include <QWidget>
#include "src/enumerates.h"

class Tool;

/**
 * @brief Widget showing grid of buttons
 *
 * Emits sendTool and sendAction when buttons are pressed.
 */
class ToolSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor: initialize layout.
    explicit ToolSelectorWidget(QWidget *parent = NULL);

    /// Size hint for layout.
    QSize sizeHint() const noexcept override;

    /// Create and add a action button in row i, column j with action defined by string.
    void addActionButton(const int i, const int j, const QString &string);

    /// Create and add a action button in row i, column j with action defined by array.
    void addActionButton(const int i, const int j, const QJsonArray &array);

    /// Create and add a tool button in row i, column j.
    /// The button takes ownership of tool.
    void addToolButton(const int i, const int j, Tool *tool);

    /// Optimal height depends on width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

signals:
    /// Send out action to master.
    void sendAction(const Action action);

    /// Send action status to action buttons.
    void sendStatus(const Action action, const int status);

    /// Send a new tool (copy of the tool of a button).
    /// Ownership of tool is transfered to receiver.
    void sendTool(Tool *tool) const;
};

#endif // TOOLSELECTORWIDGET_H
