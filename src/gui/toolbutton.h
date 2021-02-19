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

    /// Tool which remains owned by this class.
    /// Only copies of this tool are send out using sendTool.
    Tool *tool;

public:
    /// Constructor: takes ownership of tool.
    explicit ToolButton(Tool *tool, QWidget *parent = NULL) noexcept;

    /// Destruktor: deletes tool.
    virtual ~ToolButton()
    {delete tool;}

protected:
    /// Emit sendTool based on input event with adjusted device.
    virtual bool event(QEvent *event) noexcept override;

public slots:
    /// Replace tool with newtool. Old tool gets deleted.
    void setTool(Tool *newtool);

signals:
    /// ownership of tool is transfered to receiver.
    void sendTool(Tool *tool) const;
};

#endif // TOOLBUTTON_H
