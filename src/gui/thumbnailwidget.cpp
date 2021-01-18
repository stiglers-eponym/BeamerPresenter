#include "thumbnailwidget.h"

void ThumbnailWidget::generate(const PdfDocument *document)
{
    if (!document)
        document = preferences().document;
    if (renderer || !document)
        return;

    // Create the renderer without any checks.
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
    }

    const int col_width = (width()-2)/columns - 12;
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(this);
    ThumbnailButton *button;
    for (int i=0; i<document->numberOfPages(); i++)
    {
        button = new ThumbnailButton(i, this);
        connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
        const QSizeF size = document->pageSize(i);
        button->setPixmap(renderer->renderPixmap(i, col_width/size.width()));
        button->setMinimumSize(col_width, col_width*size.height()/size.width());
        layout->addWidget(button, i/columns, i%columns);
    }
    widget->setLayout(layout);
    setWidget(widget);
    QScroller::grabGesture(this);
}
