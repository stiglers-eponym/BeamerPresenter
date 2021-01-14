#include "tocwidget.h"

void TOCwidget::generateTOC(const PdfDocument *document)
{
    if (!document)
        document = preferences().document;
    if (!document || !tree_children.isEmpty())
        return;

    QVBoxLayout *layout = new QVBoxLayout();
    TOCwidget *child_widget;
    const QList<PdfOutlineEntry> &outline = document->getOutline();
    for (int i=1; i>0 && i<outline.length(); i=outline[i].next)
    {
        child_widget = new TOCwidget(outline, this, i);
        connect(child_widget, &TOCwidget::sendNavigationSignal, this, &TOCwidget::sendNavigationSignal);
        tree_children.append(child_widget);
        layout->addWidget(child_widget);
    };
    setLayout(layout);
}

TOCwidget::TOCwidget(const QList<PdfOutlineEntry> &outline, TOCwidget *parent, const int entry) :
    QWidget(parent),
    tree_parent(parent),
    page(outline[entry].page)
{
    if (std::abs(outline[entry].next) == entry + 1)
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setSpacing(0);
        layout->setMargin(0);
        QPushButton *button = new QPushButton(outline[entry].title, this);
        connect(button, &QPushButton::clicked, this, [&](){emit sendNavigationSignal(page);});
        layout->addWidget(button);
        setLayout(layout);
    }
    else
    {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(0);
        layout->setMargin(0);
        QPushButton *button = new QPushButton(outline[entry].title, this);
        connect(button, &QPushButton::clicked, this, [&](){emit sendNavigationSignal(page);});
        layout->addWidget(button);
        TOCwidget *child_widget;
        const int max = std::abs(outline[entry].next);
        for (int i=entry+1; i>0 && i<max; i=outline[i].next)
        {
            child_widget = new TOCwidget(outline, this, i);
            connect(child_widget, &TOCwidget::sendNavigationSignal, this, &TOCwidget::sendNavigationSignal);
            tree_children.append(child_widget);
            layout->addWidget(child_widget);
        };
        setLayout(layout);
    }
}

bool TOCwidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Show:
    case QEvent::FocusIn:
        if (page == -1 && tree_parent == NULL && tree_children.isEmpty())
            generateTOC();
    default:
        break;
    }
    return QWidget::event(event);
}
