// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/thumbnailwidget.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QLayoutItem>
#include <QList>
#include <QPixmap>
#include <QScroller>
#include <QShowEvent>
#include <QSizeF>
#include <QThread>
#include <algorithm>
#include <cstdlib>

#include "src/gui/thumbnailbutton.h"
#include "src/gui/thumbnailthread.h"
#include "src/master.h"
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"

/// Nearly trivial constructor.
ThumbnailWidget::ThumbnailWidget(std::shared_ptr<const PdfDocument> doc,
                                 QWidget *parent)
    : QScrollArea(parent), document(doc)
{
  setFocusPolicy(Qt::NoFocus);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  initialize();
}

ThumbnailWidget::~ThumbnailWidget()
{
  emit interruptThread();
  if (render_thread) {
    render_thread->thread()->quit();
    render_thread->thread()->wait(10000);
    delete render_thread;
  }
}

void ThumbnailWidget::initialize()
{
  debug_msg(DebugWidgets, "initializing ThumbnailWidget");
  focused_button = nullptr;
  current_page_button = nullptr;
  delete widget();
  delete layout();
  setWidget(nullptr);
  QGridLayout *layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setSpacing(0);
  QWidget *widget = new QWidget(this);
  widget->setLayout(layout);
  setWidget(widget);
  QScroller::grabGesture(this);
}

void ThumbnailWidget::showEvent(QShowEvent *event)
{
  // generate the thumbnails if necessary
  if (std::abs(ref_width - width()) > ref_width / inverse_tolerance) generate();
  // select the currently visible page
  if (!event->spontaneous()) focusPage(preferences()->page);
}

ThumbnailButton *ThumbnailWidget::buttonAtPage(int page)
{
  if (!document || page < 0 || page >= preferences()->number_of_pages ||
      !widget())
    return nullptr;
  QLayout *layout = widget()->layout();
  if (!layout) return nullptr;
  if (_flags & SkipOverlays) {
    // Get sorted list of page label indices from master document.
    const QList<int> &list = document->overlayIndices();
    if (!list.empty()) {
      QList<int>::const_iterator it =
          std::upper_bound(list.cbegin(), list.cend(), page);
      if (it != list.cbegin()) --it;
      page = it - list.cbegin();
    }
  }
  QLayoutItem *item = layout->itemAt(page);
  if (!item) return nullptr;
  return dynamic_cast<ThumbnailButton *>(item->widget());
}

void ThumbnailWidget::focusPage(const int page)
{
  if (!isVisible()) return;
  ThumbnailButton *button = buttonAtPage(page);
  if (current_page_button && current_page_button != button)
    current_page_button->clearFocus();
  if (button) {
    ensureWidgetVisible(button);
    button->giveFocus();
  }
  current_page_button = button;
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
    case Qt::Key_PageDown:
      // Page up and page down should be handle by Master,
      // not by QScrollArea. Ignore these events here.
      event->ignore();
      break;
    default:
      QScrollArea::keyPressEvent(event);
  }
}

void ThumbnailWidget::focusInEvent(QFocusEvent *event)
{
  if (focused_button)
    focused_button->giveFocus();
  else if (current_page_button)
    current_page_button->giveFocus();
  else
    focusPage(preferences()->page);
}

void ThumbnailWidget::handleAction(const Action action)
{
  if (action == PdfFilesChanged) {
    emit interruptThread();
    focused_button = nullptr;
    if (render_thread) {
      render_thread->thread()->quit();
      render_thread->thread()->wait(max_render_time_ms);
      delete render_thread;
      render_thread = nullptr;
    }
    ref_width = -100;
    initialize();
    if (isVisible()) {
      generate();
      focusPage(preferences()->page);
    }
  }
}

void ThumbnailWidget::initRenderingThread()
{
  debug_msg(DebugWidgets, "initializing rendering thread");
  render_thread = new ThumbnailThread(document);
  render_thread->moveToThread(new QThread(render_thread));
  connect(this, &ThumbnailWidget::interruptThread, render_thread,
          &ThumbnailThread::clearQueue, Qt::QueuedConnection);
  connect(this, &ThumbnailWidget::sendToRenderThread, render_thread,
          &ThumbnailThread::append, Qt::QueuedConnection);
  connect(this, &ThumbnailWidget::startRendering, render_thread,
          &ThumbnailThread::renderImages, Qt::QueuedConnection);
  connect(render_thread, &ThumbnailThread::sendThumbnail, this,
          &ThumbnailWidget::receiveThumbnail, Qt::QueuedConnection);
  render_thread->thread()->start();
  debug_msg(DebugWidgets, "started rendering thread");
}

void ThumbnailWidget::generate()
{
  debug_msg(DebugWidgets, "(re-)generating thumbnail widget" << size());
  focused_button = nullptr;
  current_page_button = nullptr;
  if (!widget()) initialize();

  emit interruptThread();
  if (!document) document = preferences()->document;
  if (!render_thread) initRenderingThread();

  QGridLayout *layout = dynamic_cast<QGridLayout *>(widget()->layout());
  if (!layout) {
    initialize();
    layout = dynamic_cast<QGridLayout *>(widget()->layout());
  }
  const int col_width =
      (viewport()->width() - (columns + 1) * layout->horizontalSpacing()) /
      columns;
  ref_width = width();
  int position = 0;
  if (_flags & SkipOverlays) {
    const QList<int> &list = document->overlayIndices();
    if (!list.empty()) {
      int link_page = list.first();
      for (auto it = list.cbegin() + 1; it != list.cend(); link_page = *it++)
        createButton(*it - 1, link_page, position++, col_width);
      createButton(document->numberOfPages() - 1, list.last(), position++,
                   col_width);
    }
  }
  if (position == 0) {
    for (; position < document->numberOfPages(); position++)
      createButton(position, position, position, col_width);
  }
  current_page_button =
      dynamic_cast<ThumbnailButton *>(layout->itemAt(0)->widget());
  emit startRendering();
}

void ThumbnailWidget::createButton(const int display_page, const int link_page,
                                   const int position, const int col_width)
{
  QGridLayout *layout = dynamic_cast<QGridLayout *>(widget()->layout());
  if (!layout) return;
  QLayoutItem *item = layout->itemAt(position);
  ThumbnailButton *button =
      item ? dynamic_cast<ThumbnailButton *>(item->widget()) : nullptr;
  if (!item || !button) {
    button = new ThumbnailButton(link_page, this);
    connect(button, &ThumbnailButton::sendNavigationSignal, master(),
            &Master::navigateToPage);
    connect(button, &ThumbnailButton::updateFocus, this,
            &ThumbnailWidget::setFocusButton);
    connect(button, &ThumbnailButton::focusUpDown, this,
            &ThumbnailWidget::moveFocusUpDown);
    layout->addWidget(button, position / columns, position % columns);
  }
  QSizeF size = document->pageSize(display_page);
  if (preferences()->default_page_part) size.rwidth() /= 2;
  button->setMinimumSize(col_width, col_width * size.height() / size.width());
  emit sendToRenderThread(
      position, (col_width - 2 * ThumbnailButton::line_width) / size.width(),
      display_page);
}

void ThumbnailWidget::receiveThumbnail(const int button_index,
                                       const QPixmap pixmap)
{
  if (pixmap.isNull() || button_index < 0) return;
  QLayout *layout = widget() ? widget()->layout() : nullptr;
  if (!layout) return;
  const auto item = layout->itemAt(button_index);
  if (!item) return;
  ThumbnailButton *button = dynamic_cast<ThumbnailButton *>(item->widget());
  if (button) button->setPixmap(pixmap);
}

void ThumbnailWidget::resizeEvent(QResizeEvent *)
{
  // Only recalculate if changes in the widget's width lie above a threshold of
  // 10%.
  if (std::abs(ref_width - width()) > ref_width / inverse_tolerance) generate();
}

void ThumbnailWidget::setFocusButton(ThumbnailButton *button)
{
  if (focused_button == button || !button) return;
  if (focused_button) focused_button->clearFocus();
  focused_button = button;
  ensureWidgetVisible(focused_button);
}

void ThumbnailWidget::moveFocusUpDown(const qint8 updown)
{
  QWidget *target = focused_button;
  if (updown > 0)
    for (int i = updown * columns; target && i > 0; --i)
      target = target->nextInFocusChain();
  else
    for (int i = updown * columns; target && i < 0; ++i)
      target = target->previousInFocusChain();
  auto button = dynamic_cast<ThumbnailButton *>(target);
  if (button) {
    focused_button->clearFocus();
    focused_button = button;
    focused_button->giveFocus();
    ensureWidgetVisible(focused_button);
  }
}
