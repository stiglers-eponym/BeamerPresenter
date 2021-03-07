#include "src/gui/tocwidget.h"
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"
#include <QGridLayout>

void TOCwidget::generateTOC(const PdfDocument *document)
{
    if (!document)
        document = preferences()->document;
    if (!document || first_button)
        return;

    const QVector<PdfDocument::PdfOutlineEntry> &outline = document->getOutline();
    // If the document outline contains no entry: outline.size() == 1
    if (outline.size() <= 1)
        return;

    QGridLayout *layout = new QGridLayout();
    QCheckBox *expand_button;
    auto add_buttons = [&](const int idx, const int depth, auto &function) -> TOCbutton*
    {
        if (std::abs(outline[idx].next) > idx + 1)
        {
            expand_button = new QCheckBox(this);
            layout->addWidget(expand_button, idx, depth, 1, depth);
        }
        else
            expand_button = NULL;
        TOCbutton *button = new TOCbutton(outline[idx].title, outline[idx].page, expand_button, this);
        layout->addWidget(button, idx, depth+1, 1, std::min(40 - depth, 20));
        connect(button, &TOCbutton::sendNavigationEvent, this, &TOCwidget::sendNavigationSignal);
        if (std::abs(outline[idx].next) - idx > 1 && idx + 1 < outline.length())
            button->tree_child = function(idx + 1, depth+1, function);
        if (outline[idx].next > 0 && outline[idx].next < outline.length())
            button->tree_next = function(outline[idx].next, depth, function);
        return button;
    };
    first_button = add_buttons(1, 0, add_buttons);
    setLayout(layout);
}

void TOCwidget::expandTo(const int page)
{
    debug_msg(DebugWidgets) << "expand to" << page;
    TOCbutton *child = first_button;
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

void TOCwidget::showEvent(QShowEvent *event)
{
    if (first_button)
        expandTo(preferences()->page);
    else
        generateTOC();
}
