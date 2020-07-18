#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QBoxLayout>
#include "src/gui/guiwidget.h"

class PixCache;

class ContainerWidget : public QWidget, public GuiWidget
{
    Q_OBJECT

    QList<GuiWidget*> child_widgets;

public:
    explicit ContainerWidget(ContainerWidget *parent = nullptr);
    const QSizeF preferedSize(QSizeF const& parent_size) const override;
    void setWidth(const qreal width) override;
    void setHeight(const qreal height) override;
    void addGuiWidget(GuiWidget* widget)
    {child_widgets.append(widget);}

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:

};

#endif // CONTAINERWIDGET_H
