#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QPushButton>
#include <QEvent>
#include <QTabletEvent>
#include <QMouseEvent>
#include <QDebug>
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/names.h"

/**
 * @brief Tool button for drawing and pointing tools.
 */
class ToolButton : public QPushButton
{
    Q_OBJECT
    Tool *tool;

public:
    explicit ToolButton(Tool *tool, QWidget *parent = NULL) noexcept;

    virtual ~ToolButton()
    {delete tool;}

protected:
    virtual bool event(QEvent *event) noexcept override;

signals:
    void sendTool(Tool *tool) const;
};

#endif // TOOLBUTTON_H
