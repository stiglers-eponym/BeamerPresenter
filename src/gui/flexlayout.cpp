// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/flexlayout.h"

#include <QVector>
#include <QWidget>
#include <algorithm>
#include <utility>

#include "src/log.h"

FlexLayout::~FlexLayout()
{
  for (const auto item : std::as_const(items)) delete item;
  items.clear();
}

QSize FlexLayout::sizeHint() const
{
  // Only aspect ratios are relevant.
  QSize hint;
  switch (direction) {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft: {
      int width = 0;
      for (const auto child : std::as_const(items)) {
        hint = child->widget()->sizeHint();
        if (child->widget()->hasHeightForWidth())
          width += hint.height() ? 4096 * hint.width() / hint.height() : 0;
        else
          width += hint.width();
      }
      debug_msg(DebugLayout, "FlexLayout size hint:" << width << 4096);
      return QSize(width, 4096);
    }
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop: {
      int height = 0;
      for (const auto child : std::as_const(items)) {
        hint = child->widget()->sizeHint();
        if (child->widget()->hasHeightForWidth())
          height += hint.width() ? 4096 * hint.height() / hint.width() : 0;
        else
          height += hint.height();
      }
      debug_msg(DebugLayout, "FlexLayout size hint:" << 4096 << height);
      return QSize(4096, height);
    }
    default:
      return QSize();
  }
}

void FlexLayout::setGeometry(const QRect &rect)
{
  debug_msg(DebugLayout, "FlexLayout setGeometry" << rect);
  QLayout::setGeometry(rect);
  if (rect.width() <= 0 || rect.height() <= 0 || items.empty()) return;
  QVector<qreal> aspects(items.size());
  QVector<int> minsizes(items.size());
  QSize hint;
  switch (direction) {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft: {
      int width = 0;
      qreal totalwidth = 0.;
      QWidget *widget;
      for (int i = 0; i < items.size(); i++) {
        widget = items[i]->widget();
        hint = widget->sizeHint();
        if (widget->hasHeightForWidth()) {
          if (hint.height() <= 0)
            aspects[i] = 1.;
          else
            aspects[i] = std::min(
                std::max(hint.width() / qreal(hint.height()), .1), 10.);
          totalwidth += aspects[i];
        } else {
          minsizes[i] = hint.width();
          width += hint.width();
        }
      }
      if (rect.width() - width > rect.height() * totalwidth) {
        // rect is wider than needed.
        const int margin =
            (rect.width() - width - rect.height() * totalwidth) / items.size();
        for (int i = 0; i < items.size(); i++) {
          if (aspects[i] > 0.) {
            items[i]->setGeometry(QRect(
                width, 0, aspects[i] * rect.height() + margin, rect.height()));
            width += aspects[i] * rect.height() + 1 + margin;
          } else {
            items[i]->setGeometry(
                QRect(width, 0, minsizes[i] + margin, rect.height()));
            width += minsizes[i] + margin;
          }
          debug_msg(DebugLayout, "set geometry:" << items[i]->geometry()
                                                 << aspects[i] << items[i]
                                                 << this);
        }
      } else {
        // rect is higher than needed.
        width = 0;
        const qreal scale =
            std::max((rect.width() - width) / (rect.height() * totalwidth), 0.);
        for (int i = 0; i < items.size(); i++) {
          if (aspects[i] > 0.) {
            items[i]->setGeometry(QRect(width, 0,
                                        scale * aspects[i] * rect.height() + .5,
                                        rect.height()));
            width += scale * aspects[i] * rect.height() + .5;
          } else {
            items[i]->setGeometry(QRect(width, 0, minsizes[i], rect.height()));
            width += minsizes[i];
          }
          debug_msg(DebugLayout, "set geometry:" << items[i]->geometry()
                                                 << aspects[i] << items[i]
                                                 << this);
        }
      }
      break;
    }
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop: {
      int height = 0;
      qreal totalheight = 0.;
      for (int i = 0; i < items.size(); i++) {
        hint = items[i]->widget()->sizeHint();
        if (items[i]->widget()->hasHeightForWidth()) {
          if (hint.width() <= 0)
            aspects[i] = 1.;
          else
            aspects[i] = std::min(
                std::max(hint.height() / qreal(hint.width()), .1), 10.);
          totalheight += aspects[i];
        } else {
          minsizes[i] = hint.height();
          height += hint.height();
        }
      }
      if (rect.height() - height > rect.width() * totalheight) {
        const int margin =
            (rect.height() - height - rect.width() * totalheight) /
            items.size();
        // rect is heigher than needed.
        for (int i = 0; i < items.size(); i++) {
          if (aspects[i] > 0.) {
            items[i]->setGeometry(QRect(0, height, rect.width(),
                                        aspects[i] * rect.width() + margin));
            height += aspects[i] * rect.width() + 1 + margin;
          } else {
            items[i]->setGeometry(QRect(0, height, rect.width(), minsizes[i]));
            height += minsizes[i] + margin;
          }
          debug_msg(DebugLayout, "set geometry:" << items[i]->geometry()
                                                 << aspects[i] << items[i]
                                                 << this);
        }
      } else {
        // rect is wider than needed.
        const qreal scale = std::max(
            (rect.height() - height) / (rect.width() * totalheight), 0.);
        height = 0;
        for (int i = 0; i < items.size(); i++) {
          if (aspects[i] > 0.) {
            items[i]->setGeometry(
                QRect(0, height, rect.width(),
                      scale * aspects[i] * rect.width() + .5));
            height += scale * aspects[i] * rect.width() + .5;
          } else {
            items[i]->setGeometry(QRect(0, height, rect.width(), minsizes[i]));
            height += minsizes[i];
          }
          debug_msg(DebugLayout, "set geometry:" << items[i]->geometry()
                                                 << aspects[i] << items[i]
                                                 << scale << this);
        }
      }
      break;
    }
  }
}
