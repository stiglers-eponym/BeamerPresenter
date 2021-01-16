#include "tocwidget.h"

void TOCwidget::generateTOC(const PdfDocument *document)
{
    if (!document)
        document = preferences().document;
    if (!document || !buttons.isEmpty())
        return;

    QGridLayout *layout = new QGridLayout();
    QCheckBox *expand_button;
    const QVector<PdfOutlineEntry> &outline = document->getOutline();
    auto add_buttons = [&](const int idx, const int depth, auto &function) -> TOCbutton*
    {
        if (std::abs(outline[idx].next) > idx + 1)
        {
            expand_button = new QCheckBox(this);
            layout->addWidget(expand_button, idx, 0, 1, depth);
        }
        else
            expand_button = NULL;
        TOCbutton *button = new TOCbutton(outline[idx].title, outline[idx].page, expand_button, this);
        buttons.append(button);
        layout->addWidget(button, idx, depth+1, 1, 12-depth);
        connect(button, &TOCbutton::sendNavigationEvent, this, &TOCwidget::sendNavigationSignal);
        if (std::abs(outline[idx].next) - idx > 1 && idx + 1 < outline.length())
            button->tree_child = function(idx + 1, depth+1, function);
        if (outline[idx].next > 0 && outline[idx].next < outline.length())
            button->tree_next = function(outline[idx].next, depth, function);
        return button;
    };
    add_buttons(1, 0, add_buttons);
    setLayout(layout);
}

bool TOCwidget::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::FocusIn:
        if (buttons.isEmpty())
            generateTOC();
        break;
    case QEvent::Show:
        if (buttons.isEmpty())
            generateTOC();
        expandTo(preferences().page);
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

void TOCwidget::expandTo(const int page)
{
    qDebug() << "expand to" << page;
    TOCbutton *child = buttons.first();
    auto expand_to = [&](TOCbutton *button, auto &function) -> void
    {
        button->expand();
        child = button->tree_child;
        if (!child || child->page > page)
            return;
        while (child && child->tree_next)
        {
            if (child->tree_next && child->tree_next->page > page)
            {
                function(child, function);
                return;
            }
            child = child->tree_next;
        }
        if (child)
            function(child, function);
    };
    while (child && child->tree_next)
    {
        if (child->tree_next && child->tree_next->page > page)
        {
            expand_to(child, expand_to);
            return;
        }
        child = child->tree_next;
    }
    if (child)
        expand_to(child, expand_to);
}
