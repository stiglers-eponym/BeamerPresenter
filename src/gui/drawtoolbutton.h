#ifndef DRAWTOOLBUTTON_H
#define DRAWTOOLBUTTON_H

#include <QPushButton>
#include <QEvent>
#include <QDebug>
#include "src/drawing/drawtool.h"

class DrawToolButton : public QPushButton
{
    Q_OBJECT
    Tool *tool;

public:
    explicit DrawToolButton(Tool *tool, QWidget *parent = nullptr) noexcept :
        QPushButton(parent), tool(tool) {setFocusPolicy(Qt::NoFocus);}

    ~DrawToolButton()
    {delete tool;}

protected:
    virtual bool event(QEvent *event) noexcept override;

signals:
    void sendTabletTool(Tool *tool) const;
    void sendTool(Tool *tool) const;
};

#endif // DRAWTOOLBUTTON_H
