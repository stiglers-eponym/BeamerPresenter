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
                emit sendToRenderThread(button, col_width/size.width(), *it-1);
                button->setMinimumSize(col_width, col_width*size.height()/size.width());
                layout->addWidget(button, i/columns, i%columns);
                i++;
            }
            const int last_page = document->numberOfPages() - 1;
            button = new ThumbnailButton(list.last(), this);
            connect(button, &ThumbnailButton::sendNavigationSignal, this, &ThumbnailWidget::sendNavigationSignal);
            const QSizeF size = document->pageSize(last_page);
            emit sendToRenderThread(button, col_width/size.width(), last_page);
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
            emit sendToRenderThread(button, col_width/size.width(), i);
            button->setMinimumSize(col_width, col_width*size.height()/size.width());
            layout->addWidget(button, i/columns, i%columns);
        }
    }
    widget->setLayout(layout);
    setWidget(widget);
    QScroller::grabGesture(this);

    emit startRendering();
}
