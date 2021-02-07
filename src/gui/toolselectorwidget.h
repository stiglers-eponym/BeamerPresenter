#ifndef TOOLSELECTORWIDGET_H
#define TOOLSELECTORWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include "src/names.h"
#include "src/gui/actionbutton.h"
#include "src/gui/toolbutton.h"

class ToolSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ToolSelectorWidget(QWidget *parent = NULL);

    QSize sizeHint() const noexcept override;

    void addActionButton(const int i, const int j, const QString &string);

    void addActionButton(const int i, const int j, const QJsonArray &array);

    void addToolButton(const int i, const int j, Tool *tool);

    bool hasHeightForWidth() const noexcept override
    {return true;}

signals:
    void sendAction(const Action action);
    void sendTool(Tool *tool) const;
};

#endif // TOOLSELECTORWIDGET_H
