#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include "src/gui/guiwidget.h"

class ContainerWidget : public QWidget, GuiWidget
{
    Q_OBJECT
public:
    explicit ContainerWidget(ContainerWidget *parent = nullptr);

signals:

};

#endif // CONTAINERWIDGET_H
