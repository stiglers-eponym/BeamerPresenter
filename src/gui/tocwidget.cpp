// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/tocwidget.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QScroller>
#include <QSizePolicy>
#include <QVector>
#include <algorithm>
#include <cstdlib>

#include "src/gui/tocbutton.h"
#include "src/log.h"
#include "src/master.h"
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"

void TOCwidget::generateTOC()
{
  if (!document) document = preferences()->document;
  if (!document || first_button) return;

  const QVector<PdfOutlineEntry> &outline = document->getOutline();
  // If the document outline contains no entry: outline.size() == 1
  if (outline.size() <= 1) return;

  QWidget *base_widget = new QWidget(this);
  base_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QGridLayout *layout = new QGridLayout();
  layout->setSizeConstraint(QGridLayout::SetNoConstraint);

  num_items = 0;
  expand_icon = QIcon::fromTheme("go-next");
  if (expand_icon.isNull()) expand_icon = QIcon::fromTheme("go-next-symbolic");
  first_button = addButtons(outline, layout, 1, 0);
  // Collaps items by default if there are more than 20 items in total.
  if (num_items > 20) {
    TOCbutton *button = first_button;
    while (button) {
      button->collapse();
      button = button->next();
    }
  }
  base_widget->setLayout(layout);
  setWidget(base_widget);
  setWidgetResizable(true);
  QScroller::grabGesture(this);
}

TOCbutton *TOCwidget::addButtons(const QVector<PdfOutlineEntry> &outline,
                                 QGridLayout *layout, int idx, const int depth)
{
  if (!layout) return nullptr;
  TOCbutton *button = nullptr;
  TOCbutton *root_button = nullptr;
  QPushButton *expand_button;
  while (idx >= 0 && idx < outline.length() && num_items <= outline.length()) {
    if (std::abs(outline[idx].next) > idx + 1) {
      if (expand_icon.isNull())
        expand_button = new QPushButton(">", this);
      else
        expand_button = new QPushButton(expand_icon, "", this);
      expand_button->setCheckable(true);
      layout->addWidget(expand_button, idx, depth, 1, 1);
    } else
      expand_button = nullptr;
    TOCbutton *tmp_button = new TOCbutton(outline[idx].title, outline[idx].page,
                                          expand_button, this);
    if (button)
      button->tree_next = tmp_button;
    else
      root_button = tmp_button;
    button = tmp_button;

    ++num_items;
    layout->addWidget(button, idx, depth + 1, 1, std::max(30 - depth, 15));
    connect(button, &TOCbutton::sendNavigationEvent, master(),
            &Master::navigateToPage);
    if (std::abs(outline[idx].next) > idx + 1)
      button->tree_child = addButtons(outline, layout, idx + 1, depth + 1);
    idx = outline[idx].next;
  }
  return root_button;
}

void TOCwidget::expandTo(const int page)
{
  debug_msg(DebugWidgets, "expand to" << page);
  TOCbutton *child = first_button;
  auto expand_to = [&](TOCbutton *button, auto &function) -> void {
    button->expand();
    child = button->tree_child;
    if (!child || child->page > page) return;
    while (child && child->tree_next) {
      if (child->tree_next && child->tree_next->page > page) {
        function(child, function);
        return;
      }
      child = child->tree_next;
    }
    if (child) function(child, function);
  };
  while (child && child->tree_next) {
    if (child->tree_next && child->tree_next->page > page) {
      expand_to(child, expand_to);
      return;
    }
    child = child->tree_next;
  }
  if (child) expand_to(child, expand_to);
}

void TOCwidget::showEvent(QShowEvent *)
{
  if (first_button)
    expandTo(preferences()->page);
  else
    generateTOC();
}

void TOCwidget::keyPressEvent(QKeyEvent *event)
{
#if (QT_VERSION_MAJOR >= 6)
  switch (event->keyCombination().toCombined())
#else
  switch (event->key() | (event->modifiers() & ~Qt::KeypadModifier))
#endif
  {
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
      event->ignore();
      break;
    default:
      QScrollArea::keyPressEvent(event);
  }
}
