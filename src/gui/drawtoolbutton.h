#ifndef DRAWTOOLBUTTON_H
#define DRAWTOOLBUTTON_H

#include <QPushButton>
#include <QEvent>
#include <QTabletEvent>
#include <QMouseEvent>
#include <QDebug>
#include "src/drawing/drawtool.h"
#include "src/names.h"

class DrawToolButton : public QPushButton
{
    Q_OBJECT
    Tool *tool;

public:
    explicit DrawToolButton(Tool *tool, QWidget *parent = nullptr) noexcept;

    virtual ~DrawToolButton()
    {delete tool;}

protected:
    virtual bool event(QEvent *event) noexcept override;

signals:
    void sendTool(Tool *tool) const;
};

#endif // DRAWTOOLBUTTON_H
