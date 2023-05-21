// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <cstdlib>
#include <algorithm>
#include <QThread>
#include <QKeyEvent>
#include <QShowEvent>
#include <QScroller>
#include <QSizeF>
#include <QList>
#include <QGridLayout>
#include <QLayoutItem>
#include <QPixmap>
#include "src/gui/thumbnailwidget.h"
#include "src/gui/thumbnailthread.h"
#include "src/gui/thumbnailbutton.h"
#include "src/rendering/pdfdocument.h"
#include "src/preferences.h"
#include "src/master.h"
#include "src/log.h"

/// inverse tolerance for widget size changes for recalculating buttons
#define INVERSE_TOLERANCE 20

void ThumbnailWidget::showEvent(QShowEvent *event)
{
    // generate the thumbnails if necessary
    generate();
    // select the currently visible page
    if (!event->spontaneous())
        focusPage(preferences()->page);
}

void ThumbnailWidget::focusPage(int page)
{
    if (page < 0 || page >= preferences()->number_of_pages)
        return;
    QLayout *layout = widget() ? widget()->layout() : nullptr;
    if (!layout)
        return;
    if (_flags & SkipOverlays)
    {
        // Get sorted list of page label indices from master document.
        const QList<int> &list = preferences()->document->overlayIndices();
        QList<int>::const_iterator it = std::upper_bound(list.cbegin(), list.cend(), page);
        if (it != list.cbegin())
            --it;
        page = it - list.cbegin();
    }
    QLayoutItem *item = layout->itemAt(page);
    if (item && item->widget())
        item->widget()->setFocus();
}

void ThumbnailWidget::setFocusIndex(ThumbnailButton *button)
{
    if (focussed_button != button)
    {
        if (focussed_button)
            focussed_button->defocus();
        focussed_button = button;
    }
}

void ThumbnailWidget::keyPressEvent(QKeyEvent *event)
{
#if (QT_VERSION_MAJOR >= 6)
    switch (event->keyCombination().toCombined())
#else
    switch (event->key() | (event->modifiers() & ~Qt::KeypadModifier))
#endif
    {
    case Qt::Key_PageUp:
        focusPage(preferences()->page - 1);
        event->ignore();
        break;
    case Qt::Key_PageDown:
        focusPage(preferences()->page + 1);
        event->ignore();
        break;
    default:
        QScrollArea::keyPressEvent(event);
    }
}

void ThumbnailWidget::handleAction(const Action action)
{
    if (action == PdfFilesChanged)
    {
        clear();
        if (render_thread)
        {
            render_thread->thread()->quit();
            render_thread->thread()->wait(2000);
            delete render_thread;
            render_thread = nullptr;
        }
    }
}

void ThumbnailWidget::generate(const PdfDocument *document)
{
    if (!document)
        document = preferences()->document;
    // Don't recalculate if changes in the widget's width lie below a threshold of 5%.
    if (!document || std::abs(ref_width - width()) < ref_width/INVERSE_TOLERANCE)
        return;

    // TODO: Correctly handle thread, this might lead to segfaults!
    clear();
    if (!render_thread)
    {
        render_thread = new ThumbnailThread(document);
        render_thread->moveToThread(new QThread(render_thread));
        connect(this, &ThumbnailWidget::interruptThread, render_thread, &ThumbnailThread::clearQueue, Qt::QueuedConnection);
        connect(this, &ThumbnailWidget::sendToRenderThread, render_thread, &ThumbnailThread::append, Qt::QueuedConnection);
        connect(this, &ThumbnailWidget::startRendering, render_thread, &ThumbnailThread::renderImages, Qt::QueuedConnection);
        connect(render_thread, &ThumbnailThread::sendThumbnail, this, &ThumbnailWidget::receiveThumbnail, Qt::QueuedConnection);
        render_thread->thread()->start();
    }

    QGridLayout *layout = new QGridLayout(this);
    debug_msg(DebugWidgets, size() << maximumViewportSize() << viewport()->size());
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    debug_msg(DebugWidgets, size() << maximumViewportSize() << viewport()->size());
    const int col_width = (viewport()->width() - (columns+1)*layout->horizontalSpacing())/columns;
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ref_width = width();
    ThumbnailButton* button;
    auto create_button = [&](const int display_page, const int link_page, const int position)
    {
        button = new ThumbnailButton(link_page, this);
        connect(button, &ThumbnailButton::sendNavigationSignal, master(), &Master::navigateToPage);
        connect(button, &ThumbnailButton::updateFocus, this, &ThumbnailWidget::setFocusIndex);
        QSizeF size = document->pageSize(display_page);
        if (preferences()->default_page_part)
            size.rwidth() /= 2;
        button->setMinimumSize(col_width, col_width*size.height()/size.width());
        layout->addWidget(button, position/columns, position%columns);
        emit sendToRenderThread(position, (col_width - 4)/size.width(), display_page);
    };
    int position = 0;
    if (_flags & SkipOverlays)
    {
        const QList<int> &list = document->overlayIndices();
        if (!list.isEmpty())
        {
            int link_page = list.first();
            for (auto it = list.cbegin()+1; it != list.cend(); link_page=*it++)
                create_button(*it-1, link_page, position++);
            create_button(document->numberOfPages()-1, list.last(), position++);
        }
    }
    if (position == 0)
    {
        for (; position < document->numberOfPages(); position++)
            create_button(position, position, position);
    }
    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setWidget(widget);
    QScroller::grabGesture(this);
    emit startRendering();
}

ThumbnailWidget::~ThumbnailWidget()
{
    clear();
    if (render_thread)
    {
        render_thread->thread()->quit();
        render_thread->thread()->wait(10000);
        delete render_thread;
    }
}

void ThumbnailWidget::receiveThumbnail(int button_index, const QPixmap pixmap)
{
    debug_msg(DebugWidgets, "receiveThumbnail:" << button_index << pixmap);
    if (pixmap.isNull() || button_index < 0)
        return;
    QLayout *layout = widget() ? widget()->layout() : nullptr;
    if (!layout)
        return;
    const  auto item = layout->itemAt(button_index);
    if (!item)
        return;
    ThumbnailButton *button = dynamic_cast<ThumbnailButton*>(item->widget());
    debug_msg(DebugWidgets, "receiveThumbnail: -> " << button);
    if (button)
        button->setPixmap(pixmap);
}
