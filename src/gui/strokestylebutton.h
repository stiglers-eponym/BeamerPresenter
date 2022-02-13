#ifndef STROKESTYLEBUTTON_H
#define STROKESTYLEBUTTON_H

#include "src/gui/toolpropertybutton.h"
#include "src/preferences.h"

/**
 * @brief Drop down menu for changing the stroke style of a draw tool.
 */
class StrokeStyleButton : public ToolPropertyButton
{
    Q_OBJECT
public:
    /// Constructor: add all items to the drop down menu.
    StrokeStyleButton(QWidget *parent = NULL);

    /// Trivial destructor.
    ~StrokeStyleButton() {}

protected:
    /// Set style of tool to selected value.
    void setToolProperty(Tool* tool) const override;

    /// Update currently selected style based on the tool for device.
    void updateTool() override
    {toolChanged(preferences()->currentTool(int(device)));}

public slots:
    /// Update currently selected tool property based on tool.
    void toolChanged(Tool *tool) override;
};

#endif // STROKESTYLEBUTTON_H
