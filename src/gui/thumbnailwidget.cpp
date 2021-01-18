#include "thumbnailwidget.h"

void ThumbnailWidget::generate(const PdfDocument *document)
{
    if (!document)
        document = preferences().document;
    // Don't recalculate if changes in the widget's width lie below a threshold of 5%.
    if (!document || std::abs(ref_width - width()) < ref_width/20)
        return;

    // Create the renderer without any checks.
    if (!renderer)
    {
        switch (preferences().renderer)
        {
#ifdef INCLUDE_POPPLER
        case AbstractRenderer::Poppler:
            renderer = new PopplerRenderer(static_cast<const PopplerDocument*>(document), preferences().page_part);
            break;
#endif
#ifdef INCLUDE_MUPDF
        case AbstractRenderer::MuPDF:
            renderer = new MuPdfRenderer(static_cast<const MuPdfDocument*>(document), preferences().page_part);
            break;
#endif
        case AbstractRenderer::ExternalRenderer:
            renderer = new ExternalRenderer(preferences().rendering_command, preferences().rendering_arguments, document, preferences().page_part);
            break;
        }

        // Check if the renderer is valid
        if (renderer == NULL || !renderer->isValid())
        {
            renderer = NULL;
            qCritical() << "Creating renderer failed" << preferences().renderer;
            return;
        }
    }
    delete widget();
    setWidget(NULL);

    const int col_width = (width()-2)/columns - 12;
    ref_width = width();
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(this);
    ThumbnailButton *button;
    int i = 0;
    if (skip_overlays)
    {
        const QList<int> &list = document->overlayIndices();
        if (!list.isEmpty())
        {
            int k = list.first();
            for (QList<int>::const_iterator it = list.cbegin()+1; it != list.cend(); k=*it++)
            {
                button = new ThumbnailButton(k, this);
                connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
                const QSizeF size = document->pageSize(*it-1);
                button->setPixmap(renderer->renderPixmap(*it-1, col_width/size.width()));
                button->setMinimumSize(col_width, col_width*size.height()/size.width());
                layout->addWidget(button, i/columns, i%columns);
                i++;
            }
            const int last_page = document->numberOfPages() - 1;
            button = new ThumbnailButton(list.last(), this);
            connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
            const QSizeF size = document->pageSize(last_page);
            button->setPixmap(renderer->renderPixmap(last_page, col_width/size.width()));
            button->setMinimumSize(col_width, col_width*size.height()/size.width());
            layout->addWidget(button, i/columns, i%columns);
            i++;
        }
    }
    if (i == 0)
    {
        for (; i<document->numberOfPages(); i++)
        {
            button = new ThumbnailButton(i, this);
            connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
            const QSizeF size = document->pageSize(i);
            button->setPixmap(renderer->renderPixmap(i, col_width/size.width()));
            button->setMinimumSize(col_width, col_width*size.height()/size.width());
            layout->addWidget(button, i/columns, i%columns);
        }
    }
    widget->setLayout(layout);
    setWidget(widget);
    QScroller::grabGesture(this);
}
