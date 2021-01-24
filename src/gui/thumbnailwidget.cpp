#include "thumbnailwidget.h"

void ThumbnailWidget::generate(const PdfDocument *document)
{
    if (!document)
        document = preferences().document;
    // Don't recalculate if changes in the widget's width lie below a threshold of 5%.
    if (!document || std::abs(ref_width - width()) < ref_width/20)
        return;

    if (!render_thread)
    {
        render_thread = new ThumbnailThread(document);
        render_thread->moveToThread(new QThread());
        connect(this, &ThumbnailWidget::sendToRenderThread, render_thread, &ThumbnailThread::append, Qt::QueuedConnection);
        connect(this, &ThumbnailWidget::startRendering, render_thread, &ThumbnailThread::renderImages, Qt::QueuedConnection);
        connect(render_thread, &ThumbnailThread::sendThumbnail, this, &ThumbnailWidget::receiveThumbnail, Qt::QueuedConnection);
        render_thread->thread()->start();
    }

    delete widget();
    setWidget(NULL);

    const int col_width = (width()-2)/columns - 12;
    ref_width = width();
    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(this);
    ThumbnailButton *button;
    auto create_button = [&](const int display_page, const int link_page, const int position)
    {
        button = new ThumbnailButton(link_page, this);
        connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
        QSizeF size = document->pageSize(display_page);
        if (preferences().default_page_part)
            size.rwidth() /= 2;
        emit sendToRenderThread(button, col_width/size.width(), display_page);
        button->setMinimumSize(col_width, col_width*size.height()/size.width());
        layout->addWidget(button, position/columns, position%columns);
    };
    int position = 0;
    if (skip_overlays)
    {
        const QList<int> &list = document->overlayIndices();
        if (!list.isEmpty())
        {
            int link_page = list.first();
            for (QList<int>::const_iterator it = list.cbegin()+1; it != list.cend(); link_page=*it++)
                create_button(*it-1, link_page, position++);
            create_button(document->numberOfPages()-1, list.last(), position++);
        }
    }
    if (position == 0)
    {
        for (; position<document->numberOfPages(); position++)
            create_button(position, position, position);
    }
    widget->setLayout(layout);
    setWidget(widget);
    QScroller::grabGesture(this);

    emit startRendering();
}
