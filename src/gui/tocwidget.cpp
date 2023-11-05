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
  QPushButton *expand_button;
  int num_items = 0;
  QIcon icon = QIcon::fromTheme("go-next");
  if (icon.isNull()) icon = QIcon::fromTheme("go-next-symbolic");
  auto add_buttons = [&](const int idx, const int depth,
                         auto &function) -> TOCbutton * {
    if (idx > outline.length() || num_items++ > outline.length())
      return nullptr;
    if (std::abs(outline[idx].next) > idx + 1) {
      if (icon.isNull())
        expand_button = new QPushButton(">", this);
      else
        expand_button = new QPushButton(icon, "", this);
      expand_button->setCheckable(true);
      layout->addWidget(expand_button, idx, depth, 1, 1);
    } else
      expand_button = nullptr;
    TOCbutton *button = new TOCbutton(outline[idx].title, outline[idx].page,
                                      expand_button, this);
    layout->addWidget(button, idx, depth + 1, 1, std::max(30 - depth, 15));
    connect(button, &TOCbutton::sendNavigationEvent, master(),
            &Master::navigateToPage);
    if (std::abs(outline[idx].next) - idx > 1 && idx + 1 < outline.length())
      button->tree_child = function(idx + 1, depth + 1, function);
    if (outline[idx].next > 0 && outline[idx].next < outline.length())
      button->tree_next = function(outline[idx].next, depth, function);
    return button;
  };
  first_button = add_buttons(1, 0, add_buttons);
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
