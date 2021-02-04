#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QPushButton>
#include <QEvent>
#include <QTabletEvent>
#include <QMouseEvent>
#include <QDebug>
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/tooldialog.h"

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

public slots:
    void setTool(Tool *newtool);

signals:
    void sendTool(Tool *tool) const;
};

#endif // TOOLBUTTON_H
