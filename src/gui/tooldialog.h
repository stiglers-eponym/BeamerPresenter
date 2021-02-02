#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"

class ToolDialog : public QDialog
{
    Q_OBJECT

public:
    ToolDialog(QWidget *parent = NULL);

    static Tool *selectTool(Tool *oldtool = NULL);
};

#endif // TOOLDIALOG_H
