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

/// inverse tolerance for widget size changes for recalculating buttons
#define INVERSE_TOLERANCE 10

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

void ThumbnailWidget::initialize()
{
  delete widget();
  delete layout();
  setWidget(nullptr);
  QGridLayout *layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  QWidget *widget = new QWidget(this);
  widget->setLayout(layout);
  setWidget(widget);
  QScroller::grabGesture(this);
}

void ThumbnailWidget::showEvent(QShowEvent *event)
{
  // generate the thumbnails if necessary
  if (std::abs(ref_width - width()) > ref_width / INVERSE_TOLERANCE) generate();
  // select the currently visible page
  if (!event->spontaneous()) focusPage(preferences()->page);
}

void ThumbnailWidget::focusPage(int page)
{
  if (!document || page < 0 || page >= preferences()->number_of_pages) return;
  QLayout *layout = widget() ? widget()->layout() : nullptr;
  if (!layout) return;
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
  if (!item) return;
  ThumbnailButton *button = dynamic_cast<ThumbnailButton *>(item->widget());
  if (!button || button == focused_button) return;
  if (focused_button) focused_button->defocus();
  focused_button = button;
  focused_button->giveFocus();
  ensureWidgetVisible(focused_button);
}

void ThumbnailWidget::setFocusButton(ThumbnailButton *button)
{
  if (focused_button != button) {
    if (focused_button) focused_button->defocus();
    focused_button = button;
    ensureWidgetVisible(focused_button);
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
  if (action == PdfFilesChanged) {
    emit interruptThread();
    focused_button = nullptr;
    if (render_thread) {
      render_thread->thread()->quit();
      render_thread->thread()->wait(2000);
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
}

void ThumbnailWidget::generate()
{
  if (!widget()) initialize();

  emit interruptThread();
  if (!document) document = preferences()->document;
  if (!render_thread) initRenderingThread();

  QGridLayout *layout = static_cast<QGridLayout *>(widget()->layout());
  const int col_width =
      (viewport()->width() - (columns + 1) * layout->horizontalSpacing()) /
      columns;
  ref_width = width();
  ThumbnailButton *button;
  QLayoutItem *item;
  auto create_button = [&](const int display_page, const int link_page,
                           const int position) {
    item = layout->itemAt(position);
    if (item) button = dynamic_cast<ThumbnailButton *>(item->widget());
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
    emit sendToRenderThread(position, (col_width - 4) / size.width(),
                            display_page);
  };
  int position = 0;
  if (_flags & SkipOverlays) {
    const QList<int> &list = document->overlayIndices();
    if (!list.empty()) {
      int link_page = list.first();
      for (auto it = list.cbegin() + 1; it != list.cend(); link_page = *it++)
        create_button(*it - 1, link_page, position++);
      create_button(document->numberOfPages() - 1, list.last(), position++);
    }
  }
  if (position == 0) {
    for (; position < document->numberOfPages(); position++)
      create_button(position, position, position);
  }
  emit startRendering();
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

void ThumbnailWidget::receiveThumbnail(int button_index, const QPixmap pixmap)
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
  if (std::abs(ref_width - width()) > ref_width / INVERSE_TOLERANCE) generate();
}

void ThumbnailWidget::moveFocusUpDown(const char updown)
{
  QWidget *target = focused_button;
  if (updown > 0)
    for (int i = 0; target && i < columns; ++i)
      target = target->nextInFocusChain();
  else
    for (int i = 0; target && i < columns; ++i)
      target = target->previousInFocusChain();
  auto button = dynamic_cast<ThumbnailButton *>(target);
  if (button) {
    focused_button->defocus();
    focused_button = button;
    focused_button->giveFocus();
    ensureWidgetVisible(focused_button);
  }
}
